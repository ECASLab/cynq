/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *
 */
#pragma once
#include <cynq/enums.hpp>
#include <cynq/status.hpp>

#include <cynq/execution-graph.hpp>

#include <memory>
#include <vector>

namespace cynq {
/**
 * @brief ExecutionStream Implementation
 *
 * This implementation is used to create execution graphs for asynchronous
 * running in a linear queue fashion, quite similar to CUDA Streams.
 *
 * All functions and their arguments added to the ExecutionStream must be
 * accesible all the time that the graph is active. Otherwise, it may lead to
 * catastrophic errors.
 */
class ExecutionStream : public IExecutionGraph {
 public:
  /**
   * @brief Construct a new execution stream.
   *
   * @param params parameters of the stream.
   */
  explicit ExecutionStream(std::shared<ExecutionGraphParameters> params);

  /**
   * @brief Adds a function to the execution stream
   *
   * This adds a new function to the graph for further execution. It is
   * enqueued last in a queue. You cannot specify the dependencies since
   * it is implemented through an execution queue
   *
   * @param function auxiliar function to add for execution. It is a lambda
   * function with all elements passed by value (recommended) and all the
   * variables used must be reachable.
   * @param dependencies unused since it is implemented in a FIFO fashion.
   * @return NodeID id of the newly added node. If the NodeID is -1, it means
   * that the function could not be added.
   */
  NodeID Add(const IExecutionGraph::Function &function,
             const std::vector<IExecutionGraph::NodeID> dependencies =
                 std::vector<IExecutionGraph::NodeID>(0)) override;

  /**
   * @brief Synchronises the execution of the stream
   *
   * It synchronises the execution of the stream partially or completely. This
   * is a blocking call, meaning that it will wait until the execution is
   * completed. If the node passed by argument already executed, it returns
   * immediately. Otherwise, it will wait until a notification of completion.
   *
   * @param node wait until the node is completed (defaults to: -1), which
   * means that it will block until the entire stream execution is completed.
   * @return Status
   */
  Status Sync(const IExecutionGraph::NodeID node = -1) override;

  /**
   * @brief Get the Last Error found during the execution
   *
   * It returns the last error that happened during the execution.
   *
   * @return Status status object with the error
   */
  Status GetLastError() override;

 private:
  /** Parameters of the stream */
  std::shared_ptr<ExecutionGraphParameters> params_;
};
}  // namespace cynq
