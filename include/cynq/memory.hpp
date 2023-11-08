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
/**
 * @brief Interface for standardising the API of Memory devices:
 * - no inheritors -
 *
 */
class IMemory {
 public:
  /**
   * @brief ~IMemory destructor method
   * Destroy the IMemory object
   */
  virtual ~IMemory() = default;
  /**
   * @brief Sync method
   * Synchronizes the memory in terms of transactions.
   *
   * @param type The orientation of the Synchronizaton this can be host to host
   * to device (HostToDevice) or device to host (DeviceToHost).
   *
   * @return Status
   */
  virtual Status Sync(const SyncType type) = 0;
  /**
   * @brief Size method
   * Gives the value for the memory size in bytes.
   *
   * @return size_t
   */
  virtual size_t Size() = 0;

 protected:
  /**
   * @brief GetHostAddress method
   * Get the Address that belongs to the host.
   * [Reference] shared memory pointer with reference counting.
   *
   * @return std::shared_ptr<uint8_t>
   */
  virtual std::shared_ptr<uint8_t> GetHostAddress() = 0;
  /**
   * @brief GetDeviceAddress method
   * Get the Address that belongs to the device.
   * [Reference] shared memory pointer with reference counting.
   *
   * @return std::shared_ptr<uint8_t>
   */
  virtual std::shared_ptr<uint8_t> GetDeviceAddress() = 0;
};
}  // namespace cynq
