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

// cynq headers
#include <cynq/enums.hpp>
#include <cynq/status.hpp>

namespace cynq {
/**
 * @brief Define an abstract representation of the accelerator parameters
 * with some prefilled fields
 */
struct AcceleratorParameters {
  /** Virtual destructor required for the inheritance */
  virtual ~AcceleratorParameters() = default;
};

/**
 * @brief Interface for standardising the API for any Accelerator device:
 * XRTAccelerator
 *
 */
class IAccelerator {
 public:
  /**
   * @brief ~IAccelerator destructor method
   * Destroy the IAccelerator object.
   *
   */
  virtual ~IAccelerator() = default;
  /**
   * @brief Type
   * Type of runtime supported by the IAccelerator.
   *
   */
  enum Type {
    /** No runtime */
    None = 0,
    /** Xilinx runtime */
    XRT
  };
  /**
   * @brief Start method
   * This method starts the accelerator in either once or continuous mode (with
   * the autorestart). Under the hood, this writes the control register to turn
   * on the accelerator with/without the autorestart bit.
   *
   * @param mode One of the values in the StartMode enum class
   * present in the enums.hpp file.
   *
   * @return Status
   */
  virtual Status Start(const StartMode mode) = 0;
  /**
   * @brief Stop method
   * This asynchronously turns off the accelerator by removing the autorestart
   * and start bits from the control registers. Please, note that the
   * accelerator will turn off once it finishes its current task.
   *
   * @return Status
   */
  virtual Status Stop() = 0;
  /**
   * @brief GetStatus method
   * This returns the accelerator state by using the DeviceStatus. This reads
   * the control register flags.
   *
   * @return DeviceStatus
   */
  virtual DeviceStatus GetStatus() = 0;
  /**
   * @brief Create method
   * Factory method used for creating specific subclasses of IAccelerator.
   *
   * @param impl
   * Used for establishing if the object is dependent on a runtime, use None if
   * this is not the case.
   *
   * @param addr
   * A 64 bit unsigned integer establishing the address from which the address
   * space of the accelerator starts.
   *
   * @return std::shared_ptr<IAccelerator>
   * This is a shared_ptr with reference counting, the type will depend
   * on the value of impl, the options are the following:
   * XRT -> XRTAccelerator
   * None -> nullptr
   *
   */
  static std::shared_ptr<IAccelerator> Create(IAccelerator::Type impl,
                                              const uint64_t addr);
  /**
   * @brief Write method
   * Performs a write operation to the accelerator through a register.
   *
   * @tparam T
   * Datatype used as the individual unit of information being written to the
   * device.
   *
   * @param address
   * Address of the accelerator.
   *
   * @param data
   * Raw pointer of type T used by the register to access the data.
   *
   * @param elements
   * Number of elements being written to the device.
   *
   * @return Status
   */
  template <typename T>
  Status Write(const uint64_t address, const T *data, const size_t elements) {
    return this->WriteRegister(address, reinterpret_cast<const uint8_t *>(data),
                               elements * sizeof(T));
  }
  /**
   * @brief Read method
   * Performs a write operation to the accelerator through a register.
   *
   * @tparam T
   * Datatype used as the individual unit of information being read from the
   * device.
   *
   * @param address
   * Address of the accelerator.
   *
   * @param data
   * Raw pointer of type T used by the register to access the data.
   *
   * @param elements
   * Number of elements being read from the device.
   *
   * @return Status
   */
  template <typename T>
  Status Read(const uint64_t address, T *data, const size_t elements) {
    return this->ReadRegister(address, reinterpret_cast<uint8_t *>(data),
                              elements * sizeof(T));
  }

 protected:
  /**
   * @brief WriteRegister method
   * Writes to the register of the accelerator.
   *
   * @param address a unsigned integer of 64 bits representing an address.
   *
   * @param data a pointer to a unsigned 8 bits variable which holds the
   * data to write to the register.
   *
   * @param size size in bytes of the data to write.
   *
   * @return Status
   */
  virtual Status WriteRegister(const uint64_t address, const uint8_t *data,
                               const size_t size) = 0;
  /**
   * @brief ReadRegister method
   *
   * @param address a unsiged integer of 64 bits representing an address.
   *
   * @param data a pointer to a unsigned 8 bits variable which holds the
   * data to read from the register.
   *
   * @param size size in bytes of the data to read.
   *
   * @return Status
   */
  virtual Status ReadRegister(const uint64_t address, uint8_t *data,
                              const size_t size) = 0;
};
}  // namespace cynq
