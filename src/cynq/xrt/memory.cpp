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
#include "cynq/xrt/memory.hpp"

namespace cynq {
Status XRTMemory::Sync(const SyncType /*type*/) { return Status{}; }

size_t XRTMemory::Size() { return sizeof(uint64_t); }

std::shared_ptr<uint8_t> XRTMemory::GetHostAddress() {
  return std::make_shared<uint8_t>();
}

std::shared_ptr<uint8_t> XRTMemory::GetDeviceAddress() {
  return std::make_shared<uint8_t>();
}
}  // namespace cynq
