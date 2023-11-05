/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#pragma once

#include <memory>
#include <string>

#include "cynq/enums.hpp"
#include "cynq/status.hpp"

namespace cynq {
class IHardware {
 public:
  virtual ~IHardware() {}
  virtual Status Reset();
  virtual std::shared_ptr<IDataMover> GetDataMover(const uint64_t address);
  virtual std::shared_ptr<IAccelerator> GetAcceleratorMover(
      const uint64_t address);
  static std::shared_ptr<IHardware> Create(const HardwareArchitecture hw,
                                           const std::string &bitstream,
                                           const std::string &xclbin);
}
}  // namespace cynq
