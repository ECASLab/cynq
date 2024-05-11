/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *
 */
#pragma once
#include <condition_variable>  // NOLINT
#include <cynq/enums.hpp>
#include <cynq/status.hpp>
#include <functional>
#include <memory>
#include <mutex>  // NOLINT
#include <string>
#include <vector>

namespace cynq {
/**
 * @brief Define an abstract representation of the IExecutionGraph parameters
 * with some prefilled fields
 */
struct ExecutionGraphParameters {
  /** Name of the stream */
  std::string name;
  /** Timeout in microseconds: time waited until having a new element before
      checking again. It has low impact */
  uint64_t timeout = 100;
  /** Virtual destructor required for the inheritance */
  virtual ~ExecutionGraphParameters() = default;
};

/**
 * @brief Execution Graph Interface
 *
 * This interface is used to create execution graphs for asynchronous running.
 * There are several possible implementations, such as execution streams (like
 * the ones from CUDA) or graphs, where you can have nodes that depend on two
 * different tasks.
 *
 * In the future, we plan to add support for Events. In the mean time, you
 * can synchronise using the Sync(node_id) for synchronising at a certain point
 * of the execution.
 *
 * All functions and their arguments added to the ExecutionGraph must be
 * accesible all the time that the graph is active. Otherwise, it may lead to
 * catastrophic errors.
 *
 * All implementations of this abstract interface must safeguard the platform
 * independency and must not have any implementation detail.
 */
class IExecutionGraph {
 public:
  /**
   * @brief Underlying type for the NodeID
   */
  typedef int NodeID;

  /**
   * @brief Underlying type for the auxiliar functions
   *
   * Auxiliar functions are lambda-like functions that run the code of interest.
   * This requires to all the members to be passed by value to ensure certain
   * integrity. However, all objects must be reachable by all the program,
   * especially if they are pointers.
   */
  typedef std::function<Status()> Function;

  /**
   * @brief Enum with the multiple implementations of the IExecutionGraph
   */
  enum Type {
    /** No runtime. It is left for the future */
    None = 0,
    /** Stream or queue based graph implementation */
    STREAM
  };

  /**
   * @brief Adds a function to the execution graph
   *
   * This adds a new function to the graph for further execution. It is
   * likely to be enqueued last in a queue. You can specify the dependencies
   * it must have, so the data dependencies can be respected.
   *
   * @param function auxiliar function to add for execution. It is a lambda
   * function with all elements passed by value (recommended) and all the
   * variables used must be reachable.
   * @param dependencies dependency nodes of the graph. It defaults to nothing
   * meaning that it depends on the last element of the queue or the data
   * dependency is already thought by design.
   * @return NodeID id of the newly added node. If the NodeID is -1, it means
   * that the function could not be added. One possible reason is that the node
   * of interest never existed. If the dependencies are already executed, the
   * node will be enqueue as a dangling node or enqueued last.
   */
  virtual IExecutionGraph::NodeID Add(
      const Function &function,
      const std::vector<NodeID> dependencies = std::vector<NodeID>(0)) = 0;

  /**
   * @brief Synchronises the execution of the graph
   *
   * It synchronises the execution of the graph partially or completely. This
   * is a blocking call, meaning that it will wait until the execution is
   * completed. If the node passed by argument already executed, it returns
   * immediately. Otherwise, it will wait until a notification of completion.
   *
   * @param node wait until the node is completed (defaults to: -1), which
   * means that it will block until the entire graph execution is completed.
   * @return Status
   */
  virtual Status Sync(const NodeID node = -1) = 0;

  /**
   * @brief Get the Last Error found during the execution
   *
   * It returns the last error that happened during the execution.
   *
   * @return Status status object with the error
   */
  virtual Status GetLastError() = 0;

  /**
   * Default destructor
   */
  virtual ~IExecutionGraph() = default;

  /**
   * @brief Factory method to create a new implementation
   *
   * @param type type of implementation given by IExecutionGraph::Type
   * @param params parameters passed to the implementation's constructor
   * @return std::shared_ptr<IExecutionGraph> new IExecutionGraph implementation
   */
  static std::shared_ptr<IExecutionGraph> Create(
      const IExecutionGraph::Type type,
      const std::shared_ptr<ExecutionGraphParameters> params);

  /**
   * @brief Node structure to hold information about each node in a generic
   * manner
   */
  struct Node {
    /** ID of the node */
    NodeID id;
    /** Auxiliar function to execute by the node */
    Function function;
    /** Dependencies of the node to be executed before the current one */
    std::vector<NodeID> dependencies = {};
    /** Pointers to the parent nodes with the IDs of dependencies */
    std::vector<Node *> parents = {};
    /** Pointers to the children nodes */
    std::vector<Node *> children = {};
  };
};
}  // namespace cynq
