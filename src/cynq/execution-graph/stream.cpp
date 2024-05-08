/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#include <chrono>              // NOLINT
#include <condition_variable>  // NOLINT
#include <cynq/execution-graph/stream.hpp>
#include <mutex>  // NOLINT
#include <queue>
#include <thread>  // NOLINT

namespace cynq {

/**
 * @brief Define the parameters for the ExecutionStream class
 */
struct ExecutionStreamParameters : public ExecutionGraphParameters {
  /** Execution queue */
  std::queue<IExecutionGraph::Node> stream_queue;
  /** Mutex for the queue */
  std::mutex stream_mutex;
  /** Mutex for the queue synchronisation */
  std::mutex stream_sync_mutex;
  /** Condition variable for empty checking */
  std::condition_variable stream_condition;
  /** Condition variable used for synchronisation */
  std::condition_variable stream_sync_condition;
  /** Current ID to add */
  IExecutionGraph::NodeID current_id = 0;
  /** Stream thread */
  std::thread stream_thread;
  /** Last error */
  Status last_error;
  /** Terminate the worker */
  bool stream_terminate = false;
  /** Flag to indicate that it is running */
  bool running = false;
  /** Virtual destructor required for the inheritance */
  virtual ~ExecutionStreamParameters() = default;
};

ExecutionStream::ExecutionStream(
    std::shared_ptr<ExecutionGraphParameters> params)
    : params_{std::make_shared<ExecutionStreamParameters>()} {
  /* Copy assigment */
  if (params) {
    *params_ = *params;
  }
  auto internal_params =
      std::dynamic_pointer_cast<ExecutionStreamParameters>(this->params_);

  internal_params->current_id = 0;
  internal_params->stream_terminate = false;
  internal_params->running = false;
  internal_params->stream_thread = std::thread(&ExecutionStream::Worker, this);
}

IExecutionGraph::NodeID ExecutionStream::Add(
    const IExecutionGraph::Function& function,
    const std::vector<IExecutionGraph::NodeID> /*dependencies*/) {
  auto params =
      std::dynamic_pointer_cast<ExecutionStreamParameters>(this->params_);
  NodeID ret;
  {
    /* Safe scope */
    std::scoped_lock lock(params->stream_mutex);

    /* Construct a new node: others are not needed */
    IExecutionGraph::Node node{};
    ret = params->current_id++;
    node.id = ret;
    node.function = function;

    /* Enqueue for work */
    params->stream_queue.push(node);
  }

  /* Notify in case the worker was empty */
  params->stream_condition.notify_one();
  return ret;
}

Status ExecutionStream::Sync(const IExecutionGraph::NodeID node) {
  NodeID current_id = -1;
  NodeID target_id = -1;
  NodeID executing_id = -1;
  bool empty = true;
  bool running = true;

  auto params =
      std::dynamic_pointer_cast<ExecutionStreamParameters>(this->params_);

  params->stream_mutex.lock();
  current_id = params->current_id;
  empty = params->stream_queue.empty();
  executing_id = params->stream_queue.front().id;
  params->stream_mutex.unlock();

  /* Filter the input argument */
  if (node >= current_id) {
    /* Exit when giving a node ID greater than the present in the queue */
    return Status{Status::INVALID_PARAMETER, "The node ID is invalid"};
  } else if (node == -1) {
    /* In case of not defining it, just place the last one */
    target_id = current_id - 1;
  } else if (empty) {
    /* In case the queue is empty */
    return Status{Status::OK, "No pending actions"};
  } else {
    target_id = node;
  }

  /* Synchronise */
  while (executing_id <= target_id && running) {
    std::unique_lock<std::mutex> lk(params->stream_sync_mutex);
    params->stream_sync_condition.wait_for(
        lk, std::chrono::microseconds(params->timeout));

    params->stream_mutex.lock();
    if (!params->stream_queue.empty()) {
      executing_id = params->stream_queue.front().id - 1;
    } else {
      executing_id = params->current_id - 1;
    }
    running = params->running;
    params->stream_mutex.unlock();
  }

  return Status{Status::OK, "Synchronisation successful"};
}

Status ExecutionStream::GetLastError() {
  Status ret{};

  auto params =
      std::dynamic_pointer_cast<ExecutionStreamParameters>(this->params_);

  {
    /* Safe scope */
    std::scoped_lock lock(params->stream_mutex);
    ret = params->last_error;
  }

  return ret;
}

void ExecutionStream::Worker() {
  bool finish = false;
  auto params =
      std::dynamic_pointer_cast<ExecutionStreamParameters>(this->params_);

  while (!finish) {
    bool empty = false;
    IExecutionGraph::Node node{};
    node.id = -1;

    /* Check if there is a node: if there is no node, waits until the condition
       variable with a timeout. Once the notification is done, it reiterates
       again. If it is not empty, just proceed */

    params->stream_mutex.lock();
    empty = params->stream_queue.empty();
    params->running = !empty;
    params->stream_mutex.unlock();

    if (empty) {
      std::unique_lock<std::mutex> lk(params->stream_mutex);
      params->stream_condition.wait_for(
          lk, std::chrono::microseconds(params->timeout));
    } else {
      std::scoped_lock<std::mutex> lk(params->stream_mutex);
      node = params->stream_queue.front();
      params->stream_queue.pop();
    }

    /* Execute the function inside */
    if (node.id != -1) {
      Status ret = node.function();
      if (Status::OK != ret.code) {
        std::scoped_lock<std::mutex> lk(params->stream_mutex);
        params->last_error = ret;
      }
    }

    /* Check for termination */
    params->stream_mutex.lock();
    finish = params->stream_terminate;
    params->stream_mutex.unlock();
    params->stream_sync_condition.notify_one();
  }
}

ExecutionStream::~ExecutionStream() {
  auto params =
      std::dynamic_pointer_cast<ExecutionStreamParameters>(this->params_);

  {
    /* Safe scope */
    std::scoped_lock lock(params->stream_mutex);
    params->stream_terminate = true;
  }

  params->stream_thread.join();
}
}  // namespace cynq
