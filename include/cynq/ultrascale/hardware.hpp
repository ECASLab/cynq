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

#include "cynq/amd/accelerator.hpp"
#include "cynq/enums.hpp"
#include "cynq/hardware.hpp"
#include "cynq/status.hpp"
#include "cynq/xrt/datamover.hpp"

namespace cynq {
/**
 * @brief UltraScale class
 * Provides an interface to access IP Cores in Xilinx FPGAs.
 *
 */
class UltraScale : public IHardware {
 public:
  /**
   * @brief ~UltraScale destructor method
   * Destroy the UltraScale object.
   */
  virtual ~UltraScale() = default;
  /**
   * @brief Reset method
   * Sets the UltraScale instance to its initial state.
   *
   * @return Status
   */
  Status Reset() override;
  /**
   * @brief GetDataMover method
   * Used for accessing the XRTDatamover instance of the UltraScale object.
   *
   * @param address a unsigned integer of 64 bits representing an address.
   *
   * @return std::shared_ptr<XRTDataMover>
   * Returns an XRTDataMover pointer with reference counting. It should be
   * thread-safe.
   *
   */
  std::shared_ptr<XRTDataMover> GetDataMover(const uint64_t address) override;
  /**
   * @brief GetAccelerator method
   * Instance of AmdAccelerator inheritors separating the hardware
   * logic from the specific logic of the accelerator.
   *
   * @param address a unsigned integer of 64 bits representing an address.
   *
   * @return std::shared_ptr<AmdAccelerator>
   * Returns an AmdAccelerator pointer with reference counting. It should be
   * thread-safe.
   *
   */
  std::shared_ptr<AmdAccelerator> GetAccelerator(
      const uint64_t address) override;
}
}  // namespace cynq
