/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */

#include <cynq/xrt/accelerator.hpp>

#include <cynq/accelerator.hpp>
#include <cynq/enums.hpp>
#include <cynq/status.hpp>

namespace cynq {
Status XRTAccelerator::Start(const StartMode /*mode*/) { return Status{}; }

Status XRTAccelerator::Stop() { return Status{}; }

DeviceStatus XRTAccelerator::GetStatus() { return DeviceStatus::Idle; }

Status XRTAccelerator::WriteRegister(const uint64_t /*address*/,
                                     const uint8_t* /*data*/,
                                     const size_t /*size*/) {
  return Status{};
}

Status XRTAccelerator::ReadRegister(const uint64_t /*address*/,
                                    uint8_t* /*data*/, const size_t /*size*/) {
  return Status{};
}
}  // namespace cynq
