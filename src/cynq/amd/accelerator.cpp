/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include "cynq/accelerator.hpp"

#include "cynq/enums.hpp"
#include "cynq/status.hpp"

using cynq::AmdAccelerator;
using cynq::DeviceStatus;
using cynq::Status;

Status AmdAccelerator::Start(const StartMode mode) { return Status::OK; }

Status AmdAccelerator::Stop() { return Status::OK; }

DeviceStatus AmdAccelerator::GetStatus() { return DeviceStatus::Idle; }

Status AmdAccelerator::Write(const uint64_t address, const uint8_t data,
                             const size_t size) {
  return Status::OK;
}

Status AmdAccelerator::Read(const uint64_t address, const uint8_t *data,
                            const size_t size) {
  return Status::OK;
}

Status AmdAccelerator::WriteRegister(const uint64_t address, uint8_t data,
                                     const size_t size) {
  return Status::OK;
}

Status AmdAccelerator::ReadRegister(const uint64_t address, uint8_t *data,
                                    const size_t size) {
  return Status::OK;
}
