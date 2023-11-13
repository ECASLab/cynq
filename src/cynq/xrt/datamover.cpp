/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include "cynq/datamover.hpp"

#include <memory>

#include "cynq/enums.hpp"
#include "cynq/memory.hpp"
#include "cynq/status.hpp"
#include "cynq/xrt/memory.hpp"

using cynq::MemoryType;
using cynq::Status;

std::shared_ptr<IMemory> XRTDataMover::GetBuffer(const size_t size,
                                                 const MemoryType type) {
  uint8_t device = 0xa0010000;
  uint8_t host = 0xa0000000;
  uint8_t* d = device;
  uint8_t* h = host;
  return IMemory::Create(IMemory::XRT, size, h, d);
}

Status XRTDataMover::Upload(const std::shared_ptr<XRTMemory> mem,
                            const size_t size, const ExecutionType exetype) {
  return Status::OK;
}

Status XRTDataMover::Download(const std::shared_ptr<XRTMemory> mem,
                              const size_t size, const ExecutionType exetype) {
  return Status::OK;
}

Status XRTDataMover::Sync() { return Status::OK; }

DeviceStatus XRTDataMover::GetStatus() { return DeviceStatus::Idle; }
