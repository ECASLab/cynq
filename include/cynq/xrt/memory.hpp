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

#include <cynq/enums.hpp>
#include <cynq/memory.hpp>
#include <cynq/status.hpp>

namespace cynq {
class XRTMemory : public IMemory {
  /**
   * @brief XRTMemory class
   * Provides the api for configuring the memory being used by the device.
   *
   */
 public:
  XRTMemory() {}
  /**
   * @brief ~XRTMemory destructor method
   * Destroy the XRTMemory object.
   *
   */
  virtual ~XRTMemory() = default;
  /**
   * @brief Sync method
   * Synchronizes the memory in terms of transactions.
   *
   * @param type The orientation of the Synchronizaton this can be host to host
   * to device (HostToDevice) or device to host (DeviceToHost).
   *
   * @return Status
   */
  Status Sync(const SyncType type) override;
  /**
   * @brief Size method
   * Gives the value for the memory size in bytes.
   *
   * @return size_t
   */
  size_t Size() override;

 protected:
  /**
   * @brief GetHostAddress method
   * Get the Address that belongs to the host.
   * [Reference] shared memory pointer with reference counting.
   *
   * @return std::shared_ptr<uint8_t>
   */
  std::shared_ptr<uint8_t> GetHostAddress() override;
  /**
   * @brief GetDeviceAddress method
   * Get the Address that belongs to the device.
   * [Reference] shared memory pointer with reference counting.
   *
   * @return std::shared_ptr<uint8_t>
   */
  std::shared_ptr<uint8_t> GetDeviceAddress() override;
};
}  // namespace cynq
