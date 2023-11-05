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

#include "cynq/enums.hpp"
#include "cynq/status.hpp"

namespace cynq {
class IMemory {
 public:
  virtual ~IMemory() {}
  virtual Status Sync(const SyncType type);
  virtual size_t Size();

 protected:
  virtual std::shared_ptr<uint64_t> GetHostAddress();
  virtual std::shared_ptr<uint64_t> GetDeviceAddress();
}
}  // namespace cynq
