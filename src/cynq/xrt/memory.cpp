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

using cynq::Status;
using cynq::XRTMemory;

Status XRTMemory::Sync(const SyncType type) { return Status::OK; }

size_t XRTMemory::Size() { return sizeof(uint64_t); }

std::shared_ptr<uint64_t> XRTMemory::HostAddress() {
  return std::make_shared<uint64_t>();
}

std::shared_ptr<uint64_t> XRTMemory::DeviceAddress() {
  return std::make_shared<uint64_t>();
}

std::shared_ptr<uint8_t> XRTMemory::GetHostAddress() {
  return make_shared<uint8_t>();
}

std::shared_ptr<uint8_t> XRTMemory::GetDeviceAddress() {
  return make_shared<uint8_t>();
}
