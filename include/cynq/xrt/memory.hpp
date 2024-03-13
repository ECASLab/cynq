/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#pragma once

#include <cynq/dma/datamover.hpp>
#include <cynq/enums.hpp>
#include <cynq/memory.hpp>
#include <cynq/status.hpp>
#include <cynq/xrt/datamover.hpp>
#include <memory>

namespace cynq {
/**
 * @brief XRTMemory class
 * Provides the api for configuring the data buffers, this class is based
 * on the Buffer object from the xilinx runtime.
 *
 */
class XRTMemory : public IMemory {
 public:
  /**
   * @brief Construct a new XRTDataMover object
   *
   * @param size size of the memory region in bytes
   * @param hostptr host pointer where to store the host related memory. It can
   * be null in case of using moverptr.
   * @param devptr host pointer where to store the device related memory. It can
   * be null in case of using moverptr.
   * @param moverptr data mover specific metadata
   */
  XRTMemory(const std::size_t size, uint8_t* hostptr, uint8_t* devptr,
            void* moverptr);
  /**
   * @brief Default constructor
   *
   * The default constructor is deleted because the Memory depends on the
   * mover
   */
  XRTMemory() = delete;
  /**
   * @brief ~XRTMemory destructor method
   * Destroy the XRTMemory object.
   *
   */
  virtual ~XRTMemory();
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

  /** Define the friend relacionship between the mover and the memory */
  friend class DMADataMover;
  friend class XRTDataMover;

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

 private:
  /** Memory region size */
  std::size_t size_;
  /** Host memory pointer */
  uint8_t* host_ptr_;
  /** Device memory pointer */
  uint8_t* dev_ptr_;
  /** Mover metadata pointer */
  void* mover_ptr_;
};
}  // namespace cynq
