/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#pragma once
#include <cynq/enums.hpp>
#include <cynq/execution-graph.hpp>
#include <cynq/status.hpp>
#include <memory>

namespace cynq {
/**
 * @brief Interface for standardising the API of Memory devices:
 * XRTMemory
 *
 */
class IMemory {
 public:
  /**
   * @brief ~IMemory destructor method
   * Destroy the IMemory object.
   *
   */
  virtual ~IMemory() = default;
  /**
   * @brief Type
   * Type of runtime supported by the IMemory.
   *
   */

  enum Type {
    /** No runtime */
    None = 0,
    /** Xilinx runtime */
    XRT
  };
  /**
   * @brief Sync method
   * Synchronizes the memory in terms of transactions.
   *
   * @param type The orientation of the Synchronizaton this can be host to
   * host to device (HostToDevice) or device to host (DeviceToHost).
   *
   * @return Status
   */
  virtual Status Sync(const SyncType type) = 0;
  /**
   * @brief Sync method (Asynchronous)
   * This function executes asynchronously through an execution graph. Please,
   * see IMemory::Sync for more information.
   *
   * @param graph The execution graph to work on
   *
   * @param type The orientation of the Synchronizaton this can be host to
   * host to device (HostToDevice) or device to host (DeviceToHost).
   *
   * @return Status
   */
  virtual Status Sync(std::shared_ptr<IExecutionGraph> graph,
                      const SyncType type);
  /**
   * @brief Size method
   * Gives the value for the memory size in bytes.
   *
   * @return size_t
   */
  virtual size_t Size() = 0;
  /**
   * @brief HostAddress method
   * Getter for the address of the host.
   *
   * @tparam T
   * A type which is used for type casting within this method.
   *
   * @return std::shared_ptr<T>
   *
   */
  template <typename T>
  std::shared_ptr<T> HostAddress() {
    return std::reinterpret_pointer_cast<T>(this->GetHostAddress());
  }
  /**
   * @brief DeviceAddress method
   * Getter for the address of the device.
   *
   * @tparam T
   * A type which is used for type casting within this method.
   *
   * @return std::shared_ptr<T>
   *
   */
  template <typename T>
  std::shared_ptr<T> DeviceAddress() {
    return std::reinterpret_pointer_cast<T>(this->GetDeviceAddress());
  }

  /**
   * @brief Create method
   * Factory method to create specific subclasses of IMemory.
   *
   * @param impl
   * Used for establishin if the object is dependent on a runtime, use None if
   * this is not the case.
   *
   * @param size
   * Size in bytes of the memory, this defines the length of the address space
   * of the transaction being mapped.
   *
   * @param hostptr
   * Pointer of the address that belongs to the host, used for memory mapping
   * from the host to the device.
   *
   * @param devptr
   * Pointer of the address that belongs to the device, used for mapping memory
   * to the device.
   *
   * @param moverptr
   * Pointer to platform specific properties
   *
   * @return std::shared_ptr<IMemory>
   * This is a shared_ptr with reference counting, the type will depend
   * on the value of impl, the options are the following:
   * following:
   * XRT -> XRTMemory
   * None -> nullptr
   *
   */
  static std::shared_ptr<IMemory> Create(IMemory::Type impl,
                                         const std::size_t size,
                                         uint8_t* hostptr, uint8_t* devptr,
                                         void* moverptr);

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
