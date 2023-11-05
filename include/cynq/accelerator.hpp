/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#pragma once
#include "cynq/enums.hpp"
#include "cynq/status.hpp"

namespace cynq {
class IAccelerator {
 public:
  virtual ~IAccelerator() {}
  virtual Status Start(const StartMode mode);
  virtual Status Stop();
  virtual DeviceStatus GetStatus();

 protected:
  virtual Status WriteRegister(const uint64_t address, uint64_t *data,
                               const size_t size);
  virtual Status ReadRegister(const uint64_t address, uint64_t *data,
                              const size_t size);
}
}  // namespace cynq
