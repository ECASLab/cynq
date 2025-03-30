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

/* Definitions from the AXI GPIO driver documentation */
#define XGPIO_DATA_OFFSET 0x0  /**< Data register for 1st channel */
#define XGPIO_TRI_OFFSET 0x4   /**< I/O direction reg for 1st channel */
#define XGPIO_DATA2_OFFSET 0x8 /**< Data register for 2nd channel */
#define XGPIO_TRI2_OFFSET 0xC  /**< I/O direction reg for 2nd channel */
#define XGPIO_CHAN_OFFSET 8    /**< Channel offeset */

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

Status AXIGPIO::Read(const uint channel, const uint start_bit,
                     const uint stop_bit, uint32_t &value) {  // NOLINT
  uint64_t addr = channel * XGPIO_CHAN_OFFSET;
  uint32_t current_value = 0;

  /* Read the current value */
  Status st = ReadRegister(addr, reinterpret_cast<uint8_t *>(&current_value),
                           sizeof(uint32_t));

  /* Prepare the mask */
  uint32_t bits = stop_bit - start_bit; /* 3-1 = 2 */
  uint32_t mask = (1 << bits) - 1;      /* 2^2 = 4 - 1 = 0b0011 */
  current_value >>= start_bit;

  /* Filter the value */
  value = current_value & mask;
  return st;
}

Status AXIGPIO::Write(const uint channel, const uint start_bit,
                      const uint stop_bit, const uint32_t value) {  // NOLINT
  uint64_t addr = channel * XGPIO_CHAN_OFFSET;
  uint32_t current_value = 0, new_value = 0;

  /* Read the current value */
  Status st = ReadRegister(addr, reinterpret_cast<uint8_t *>(&current_value),
                           sizeof(uint32_t));
  if (st.code) return st;

  /* Prepare the mask */
  uint32_t bits = stop_bit - start_bit; /* 3-1 = 2 */
  uint32_t mask = (1 << bits) - 1;      /* 2^2 = 4 - 1 = 0b0011 */

  /* Filter the value. It needs to be negated since we are going to write
     a negative mask. Assume we are going to write a 0b10 in the bits from 1:3,
     so, we need to negate it to have 0b01 */
  new_value = mask & ~value;
  /* Then we shift and negate to have: 0b00010 -> 0b11101, so when we
     write the value using an AND, we have xx10x, where x is the original
     value of already written */
  new_value = ~(new_value << start_bit);
  new_value = current_value & new_value;

  st = WriteRegister(addr, reinterpret_cast<uint8_t *>(&new_value),
                     sizeof(uint32_t));
  return st;
}

Status AXIGPIO::Config(const uint channel, const uint start_bit,
                       const uint stop_bit,            // NOLINT
                       const AXIGPIO::PinMode mode) {  // NOLINT
  uint64_t addr = channel * XGPIO_CHAN_OFFSET + XGPIO_TRI_OFFSET;
  uint32_t config = 0, result = 0;

  /* Read the current mask */
  Status st = ReadRegister(addr, reinterpret_cast<uint8_t *>(&config),
                           sizeof(uint32_t));
  if (st.code) return st;

  uint32_t bits = stop_bit - start_bit; /* 3-1 = 2 */
  uint32_t mask = (1 << bits) - 1;      /* 2^2 = 4 - 1 = 0b0011 */
  mask <<= start_bit;                   /* 0b0011 << 1 = 0b00110 */
  /* If output: multiplies by 0, otherwise, leave as is */
  mask *= static_cast<uint32_t>(mode);
  /* In case if mask is 00100 and inputs are 00110 or 00010 */
  /* First XOR:  00010, 00110 */
  result = config ^ mask;
  /* Second XOR: 00110, 00010 (as configured in the input) */
  result ^= config;

  /* Write direction */
  st = WriteRegister(addr, reinterpret_cast<uint8_t *>(&result),
                     sizeof(uint32_t));
  return st;
}

}  // namespace cynq
