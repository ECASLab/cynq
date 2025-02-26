/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include <cynq/accelerator.hpp>
#include <cynq/enums.hpp>
#include <cynq/mmio/accelerator.hpp>
#include <cynq/mmio/axi-gpio.hpp>
#include <cynq/status.hpp>
#include <iostream>  // TODO(lleon): Remove this after implementation
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>

namespace cynq {

AXIGPIO::AXIGPIO(const uint64_t addr) : MMIOAccelerator{addr} {}

Status AXIGPIO::Start(const StartMode) {
  return Status{Status::NOT_IMPLEMENTED, "Not implemented"};
}

Status AXIGPIO::Stop() {
  return Status{Status::NOT_IMPLEMENTED, "Not implemented"};
}

Status AXIGPIO::Sync() {
  return Status{Status::NOT_IMPLEMENTED, "Not implemented"};
}

DeviceStatus AXIGPIO::GetStatus() { return DeviceStatus::Done; }

Status AXIGPIO::AttachRegister(const uint64_t, uint8_t *, const RegisterAccess,
                               const size_t) {  // NOLINT
  return Status{Status::NOT_IMPLEMENTED, "Not implemented"};
}

Status AXIGPIO::Attach(const uint64_t, std::shared_ptr<IMemory>) {
  return Status{Status::NOT_IMPLEMENTED, "Not implemented"};
}

AXIGPIO::~AXIGPIO() {}

int AXIGPIO::GetMemoryBank(const uint /* pos */) { return 0; }

Status AXIGPIO::ReadPin(const uint channel, const uint pin,
                        uint &bit) {  // NOLINT
  std::cout << "AXIGPIO: Reading channel " << channel << " at pin " << pin
            << std::endl;
  bit = 0;
  return Status{};
}

Status AXIGPIO::WritePin(const uint channel, const uint pin,
                         const uint bit) {  // NOLINT
  std::cout << "AXIGPIO: Writting channel " << channel << " at pin " << pin
            << " with value " << bit << std::endl;
  return Status{};
}

Status AXIGPIO::ConfigPin(const uint channel, const uint pin,
                          const AXIGPIO::PinMode mode) {  // NOLINT
  std::cout << "AXIGPIO: Writting channel " << channel << " at pin " << pin
            << " with mode " << static_cast<int>(mode) << std::endl;
  return Status{};
}

}  // namespace cynq
