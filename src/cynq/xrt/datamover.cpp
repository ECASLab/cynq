/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */

#include <cynq/xrt/datamover.hpp>

#include <memory>

#include <cynq/datamover.hpp>
#include <cynq/enums.hpp>
#include <cynq/memory.hpp>
#include <cynq/status.hpp>
#include <cynq/xrt/memory.hpp>

namespace cynq {
std::shared_ptr<IMemory> XRTDataMover::GetBuffer(const size_t size,
                                                 const MemoryType /*type*/) {
  uint8_t* d = nullptr;
  uint8_t* h = nullptr;
  return IMemory::Create(IMemory::XRT, size, h, d);
}

Status XRTDataMover::Upload(const std::shared_ptr<IMemory> /*mem*/,
                            const size_t /*size*/,
                            const ExecutionType /*exetype*/) {
  return Status{};
}

Status XRTDataMover::Download(const std::shared_ptr<IMemory> /*mem*/,
                              const size_t /*size*/,
                              const ExecutionType /*exetype*/) {
  return Status{};
}

Status XRTDataMover::Sync() { return Status{}; }

DeviceStatus XRTDataMover::GetStatus() { return DeviceStatus::Idle; }
}  // namespace cynq
