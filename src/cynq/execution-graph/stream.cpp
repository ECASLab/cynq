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

}  // namespace cynq
