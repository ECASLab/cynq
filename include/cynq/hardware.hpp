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
#include <string>

#include "cynq/accelerator.hpp"
#include "cynq/datamover.hpp"
#include "cynq/enums.hpp"
#include "cynq/status.hpp"

namespace cynq {
/**
 * @brief Interface for standardising the API of Hardware Devices:
 * - no inheritors -
 */
class IHardware {
 public:
  /**
   * @brief ~IHardware destructor method
   * Destroy the IHardware object.
   *
   */
  virtual ~IHardware() = default;
  /**
   * @brief Reset method
   * Sets the IHardware instance to its initial state.
   *
   * @return Status
   */
  virtual Status Reset() = 0;
  /**
   * @brief GetDataMover method
   * Used for accessing the IDataMover instance of
   * IHardware inheritors for decoupling the DataMovement separating the
   * hardware logic from the data movement logic.
   *
   * @param address a unsigned integer of 64 bits representing an address.
   *
   * @return std::shared_ptr<IDataMover>
   * Returns an IDataMover pointer with reference counting. It should be
   * thread-safe.
   *
   */
  virtual std::shared_ptr<IDataMover> GetDataMover(const uint64_t address) = 0;
  /**
   * @brief GetAcceleratorMover method
   * IDataMover instance of IAccelerator inheritors separating the hardware
   * logic from the specific logic of the accelerator.
   *
   * @param address a unsigned integer of 64 bits representing an address.
   *
   * @return std::shared_ptr<IAccelerator>
   * Returns an IAccelerator pointer with reference counting. It should be
   * thread-safe.
   *
   */
  virtual std::shared_ptr<IAccelerator> GetAcceleratorMover(
      const uint64_t address) = 0;
  /**
   * @brief Create method
   * Factory method to create a hardware-specific subclasses for accelerators
   * and data movers.
   *
   * @example
   * - no implementations -
   *
   * @param hw One of the values in the HardwareArchitecture enum class
   * present in the enums.hpp file that should correspond to the device being
   * used.
   *
   * @param bitstream string that represents the name of the file
   * with the bitstream.
   *
   * @param xclbin string that represents the name of the xcl.bin file.
   *
   * @return std::shared_ptr<IHardware>
   * Returns an IAccelerator pointer with reference counting. It should be
   * thread-safe.
   *
   */
  static std::shared_ptr<IHardware> Create(const HardwareArchitecture hw,
                                           const std::string &bitstream,
                                           const std::string &xclbin);
};
}  // namespace cynq
