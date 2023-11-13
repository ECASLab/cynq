/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#pragma once
#include <cynq/accelerator.hpp>
#include <cynq/enums.hpp>
#include <cynq/status.hpp>

namespace cynq {
/**
 * @brief XRTAccelerator class
 * This class provides the api to operate the accelerator.
 *
 */
class XRTAccelerator : public IAccelerator {
 public:
  XRTAccelerator() {}
  /**
   * @brief ~XRTAccelerator destructor method
   * Destroy the XRTAccelerator object
   *
   */
  virtual ~XRTAccelerator() = default;
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
  Status Start(const StartMode mode) override;

  /**
   * @brief Stop method
   * This asynchronously turns off the accelerator by removing the autorestart
   * and start bits from the control registers. Please, note that the
   * accelerator will turn off once it finishes its current task.
   *
   * @return Status
   */
  Status Stop() override;
  /**
   * @brief GetStatus method
   * This returns the accelerator state by using the DeviceStatus. This reads
   * the control register flags.
   *
   * @return DeviceStatus
   */
  DeviceStatus GetStatus() override;

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
  Status WriteRegister(const uint64_t address, const uint8_t *data,
                       const size_t size) override;
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
  Status ReadRegister(const uint64_t address, uint8_t *data,
                      const size_t size) override;
};
}  // namespace cynq
