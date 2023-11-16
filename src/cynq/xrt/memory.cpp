/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include "cynq/memory.hpp"

#include <memory>

#include "cynq/enums.hpp"
#include "cynq/status.hpp"
#include "cynq/xrt/datamover.hpp"
#include "cynq/xrt/memory.hpp"

namespace cynq {
XRTMemory::XRTMemory(const std::size_t size, uint8_t* hostptr, uint8_t* devptr,
                     void* moverptr)
    : size_{size}, host_ptr_{hostptr}, dev_ptr_{devptr}, mover_ptr_{moverptr} {}

Status XRTMemory::Sync(const SyncType /*type*/) { return Status{}; }

size_t XRTMemory::Size() { return sizeof(uint64_t); }

std::shared_ptr<uint8_t> XRTMemory::GetHostAddress() {
  return std::make_shared<uint8_t>();
}

std::shared_ptr<uint8_t> XRTMemory::GetDeviceAddress() {
  return std::make_shared<uint8_t>();
}

XRTMemory::~XRTMemory() {
  if (mover_ptr_) {
    auto meta = reinterpret_cast<XRTDataMoverMeta*>(mover_ptr_);
    delete meta;
  }
}
}  // namespace cynq
