/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *
 */
#include <cynq/execution-graph.hpp>
#include <cynq/execution-graph/stream.hpp>
#include <memory>

namespace cynq {
std::shared_ptr<IExecutionGraph> IExecutionGraph::Create(
    IExecutionGraph::Type impl,
    const std::shared_ptr<ExecutionGraphParameters> params) {
  switch (impl) {
    case IAccelerator::Type::STREAM:
      return std::make_shared<ExecutionStream>(params);
    default:
      return nullptr;
  }
}
}  // namespace cynq
