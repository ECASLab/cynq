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
#include <cynq/mmio/accelerator.hpp>
#include <cynq/status.hpp>
#include <memory>

namespace cynq {
/**
 * @brief AXIGPIO class
 * This class provides the api to operate the accelerator.
 *
 */
class AXIGPIO : public MMIOAccelerator {
 public:
  enum class PinMode {
    /** For pin configuration as an output */
    MODE_OUTPUT = 0,
    /** For pin configuration as an input */
    MODE_INPUT = 1,
  };

  /**
   * @brief Delete the default constructor since address is needed
   */
  AXIGPIO() = delete;
  /**
   * @brief Construct a new AXIGPIO object
   *
   * It constructs an accessor to the accelerator in the PL design according
   * to the AXI-lite memory mapping. This is widely compatible with AXI4-lite
   * controlled HLS designs.
   *
   * @param addr 64-bit address in the physical memory space
   */
  explicit AXIGPIO(const uint64_t addr);

  /**
   * @brief ~AXIGPIO destructor method
   * Destroy the AXIGPIO object
   */
  virtual ~AXIGPIO();

  /**
   * @brief Start method
   *
   * This method starts the accelerator in either once or continuous mode (with
   * the autorestart). Under the hood, this writes the control register to turn
   * on the accelerator with/without the autorestart bit.
   *
   * It also synchronises the registers, writing them if attached.
   *
   * @param mode One of the values in the StartMode enum class
   * present in the enums.hpp file.
   *
   * @return Status
   */
  Status Start(const StartMode mode) override;

  /**
   * @brief Stop method
   *
   * This asynchronously turns off the accelerator by removing the autorestart
   * and start bits from the control registers. Please, note that the
   * accelerator will turn off once it finishes its current task.
   *
   * It also synchronises the registers, reading them if attached.
   *
   * @return Status
   */
  Status Stop() override;

  /**
   * @brief Sync method
   *
   * Forces to wait until the accelerator execution is different from
   * "DeviceStatus::Running"
   *
   * It also synchronises the registers, reading them if attached.
   *
   * @return Status
   */
  Status Sync() override;

  /**
   * @brief Get the memory bank ID (not implemented)
   *
   * It corresponds to the argument memory argument for affinity. It is useful
   * for assigning memory banks to the DataMovers before requesting any memory.
   *
   * It is only used by Vitis and Alveo workflows
   *
   * @param pos memory bank position within the kernel
   *
   * @return 0
   */
  int GetMemoryBank(const uint pos) override;

  /**
   * @brief GetStatus method
   * This returns the accelerator state by using the DeviceStatus. This reads
   * the control register flags.
   *
   * @return DeviceStatus
   */
  DeviceStatus GetStatus() override;

  /**
   * @brief Attach a memory argument
   *
   * Performs an attachment of the argument and the respective pointer.
   * The use of this overload for IMemory buffers is highly recommended.
   *
   * @param index Argument position of the argument to set
   *
   * @param mem Memory buffer to attach to the argument
   *
   * @return Status
   */
  Status Attach(const uint64_t index, std::shared_ptr<IMemory> mem) override;

  /**
   * @brief Reads a pin
   *
   * It reads a single bit of the GPIO
   *
   * @param channel GPIO channel
   *
   * @param pin GPIO channel pin
   *
   * @param bit Result of the bit after read
   *
   * @return Status
   */
  Status ReadPin(const uint channel, const uint pin, uint &bit);  // NOLINT

  /**
   * @brief Writes a pin
   *
   * It writes a single bit of the GPIO
   *
   * @param channel GPIO channel
   *
   * @param pin GPIO channel pin
   *
   * @param bit Bit value to write (LSB)
   *
   * @return Status
   */
  Status WritePin(const uint channel, const uint pin,
                  const uint bit);  // NOLINT

  /**
   * @brief Configure a pin
   *
   * It configures a pin into input or output
   *
   * @param channel GPIO channel
   *
   * @param pin GPIO channel pin
   *
   * @param mode mode to configure according to AXIGPIO::PinMode
   *
   * @return Status
   */
  Status ConfigPin(const uint channel, const uint pin,
                   const PinMode mode);  // NOLINT

 protected:
  /**
   * @brief Implementation of the Attach Register method
   *
   * @param index index of the argument to set
   *
   * @param data a pointer to an unsigned 8 bits variable which holds the
   * data to read from the register.
   *
   * @param access Access type of the register
   *
   * @param size size in bytes of the data to read.
   *
   * @return Status
   */
  Status AttachRegister(const uint64_t index, uint8_t *data,
                        const RegisterAccess access,
                        const size_t size) override;
};
}  // namespace cynq
