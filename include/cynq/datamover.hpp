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
class IDataMover {
 public:
  virtual ~IDataMover() {}
  virtual std::shared_ptr<IMemory> GetBuffer(const size_t size,
                                             const MemoryType type);
  virtual Status Upload(const std::shared_ptr<IMemory> mem, const size_t size,
                        const ExecutionType exetype);
  virtual Status Sync();
  virtual DeviceStatus GetStatus();
}
}  // namespace cynq
