/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#include <cynq/execution-graph/stream.hpp>

namespace cynq {

/**
 * @brief Define the parameters for the ExecutionStream class
 */
struct ExecutionStreamParameters : public ExecutionGraphParameters {
  /** Virtual destructor required for the inheritance */
  virtual ~ExecutionStreamParameters() = default;
};

ExecutionStream::ExecutionStream(
    std::shared_ptr<ExecutionGraphParameters> params)
    : params_{std::make_shared<ExecutionStreamParameters>()} {
  /* Copy assigment */
  if (params) {
    *std::static_pointer_cast<ExecutionGraphParameters>(params_) = *params;
  }
}

IExecutionGraph::NodeID ExecutionStream::Add(
    const IExecutionGraph::Function& /*function*/,
    const std::vector<IExecutionGraph::NodeID> /*dependencies*/) {
  return -1;
}

Status ExecutionStream::Sync(const IExecutionGraph::NodeID /*node*/) {
  return Status{};
}

Status ExecutionStream::GetLastError() { return Status{}; }

ExecutionStream::~ExecutionStream() {}

}  // namespace cynq
