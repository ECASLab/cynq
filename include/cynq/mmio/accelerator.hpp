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
#include <memory>

namespace cynq {
/**
 * @brief MMIOAccelerator class
 * This class provides the api to operate the accelerator.
 *
 */
class MMIOAccelerator : public IAccelerator {
 public:
  /**
   * @brief Delete the default constructor since address is needed
   */
  MMIOAccelerator() = delete;
  /**
   * @brief Construct a new MMIOAccelerator object
   *
   * It constructs an accessor to the accelerator in the PL design according
   * to the AXI-lite memory mapping. This is widely compatible with AXI4-lite
   * controlled HLS designs.
   *
   * @param addr 64-bit address in the physical memory space
   */
  explicit MMIOAccelerator(const uint64_t addr);
  /**
   * @brief ~MMIOAccelerator destructor method
   * Destroy the MMIOAccelerator object
   */
  virtual ~MMIOAccelerator();
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

 protected:
  /**
   * @brief Write Register method
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
   * @brief Read Register method
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

 private:
  /** Accelerator address */
  uint64_t addr_;
  /** Address space size */
  uint64_t addr_space_size_;
  /** Accelerator-specific configurations */
  std::unique_ptr<AcceleratorParameters> accel_params_;
  /** Synchronises the registers attached */
  Status SyncRegisters(const SyncType type);
};
}  // namespace cynq
