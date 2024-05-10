/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <xrt.h>
#include <xrt/xrt_device.h>
#pragma GCC diagnostic pop

#include <cstdint>
#include <cynq/dma/datamover.hpp>
#include <cynq/enums.hpp>
#include <cynq/hardware.hpp>
#include <cynq/mmio/accelerator.hpp>
#include <cynq/status.hpp>
#include <memory>
#include <string>
#include <vector>

namespace cynq {
/**
 * @brief Contains the information about the registers used for
 * the clocks
 */
struct UltraScaleClocks {
  inline static constexpr int max_clocks = 4;
  /** Verifies if the PL clock is enabled */
  std::array<bool, max_clocks> pl_active = {false};
  /** Verifies if the PL clock is valid */
  std::array<bool, max_clocks> pl_valid = {false};
  /** Contains the source clock frequency */
  std::array<float, max_clocks> src_freq = {0.f};
  /** Contains the information about the queried pl registers */
  std::array<uint32_t, max_clocks> pl_reg;
  /** Contains the information about the queried source registers */
  std::array<uint32_t, max_clocks> src_reg;
  /** Target clocks: used in UltraScale+ */
  std::array<float, 4> target_clocks_mhz = {-1.f};
  /** Current clocks: used in UltraScale+ */
  std::array<float, 4> current_clocks_mhz = {-1.f};
};

/**
 * @brief Specialisation of the parameters given by the UltraScale. It
 * is based on the PYNQ and XRT
 */
struct UltraScaleParameters : public HardwareParameters {
  /** XRT Device linked to the FPGA */
  xrt::device device_;
  /** XRT class representing the xclbin object */
  xrt::xclbin xclbin_;
  /** Information regarding the clocks */
  UltraScaleClocks clocks_;
  /** Virtual destructor required for the inheritance */
  virtual ~UltraScaleParameters() = default;
};

/**
 * @brief UltraScale class
 * Provides an interface to access IP Cores in Xilinx FPGAs, the compatible
 * devices are the following: ZCU102, ZCU106, K26.
 *
 * This class DO NOT have the support for XCLBIN kernels YET and it takes into
 * account the Vivado workflow only
 */
class UltraScale : public IHardware {
 public:
  /**
   * @brief Construct a new UltraScale object
   *
   * Configure the FPGA with an overlay (bitstream) or a xclbin object. The
   * configuration files are mutually exclusive. If using a bitstream, the
   * xclbin must be the default one. If no bitstream passed (empty), the xclbin
   * file is mandatory.
   *
   * @param bitstream_file full path to the bitstream object (.bit file)
   * @param xclbin_file full path to the xclbin object (use the default one
   * in the third-party/resources).
   */
  UltraScale(const std::string &bitstream_file, const std::string &xclbin_file);
  /**
   * No default constructor required
   */
  UltraScale() = delete;
  /**
   * @brief ~UltraScale destructor method
   * Destroy the UltraScale object.
   */
  virtual ~UltraScale();
  /**
   * @brief Reset method
   * Sets the UltraScale instance to its initial state.
   *
   * @return Status
   */
  Status Reset() override;
  /**
   * @brief GetDataMover method
   * Used for accessing the IDataMover instance of the UltraScale object.
   *
   * @param address a unsigned integer of 64 bits representing an address.
   *
   * @return std::shared_ptr<IDataMover>
   * Returns an IDataMover pointer with reference counting. It should be
   * thread-safe.
   *
   */
  std::shared_ptr<IDataMover> GetDataMover(const uint64_t address) override;
  /**
   * @brief GetAccelerator method (overload - not implemented)
   * Do not use this method since it is not implemented and it will lead
   * to a nullptr
   *
   * @param kernelname kernel name for XRT kernel (not used)
   *
   * @return std::shared_ptr<IAccelerator> nullptr
   *
   */
  std::shared_ptr<IAccelerator> GetAccelerator(
      const std::string &kernelname) override;

  /**
   * @brief GetAccelerator method
   * Instance of IAccelerator inheritors separating the hardware
   * logic from the specific logic of the accelerator.
   *
   * @param address a unsigned integer of 64 bits representing an address.
   *
   * @return std::shared_ptr<IAccelerator>
   * Returns an IAccelerator pointer with reference counting. It should be
   * thread-safe.
   *
   */
  std::shared_ptr<IAccelerator> GetAccelerator(const uint64_t address) override;
  /**
   * @brief Queries the device looking for its characteristics.
   *
   * To be defined in future releases.
   *
   * @return Status
   */
  Status DeviceQuery();
  /**
   * @brief Queries the kernels available in the design
   *
   * To be defined in future releases.
   *
   * @return Status
   */
  Status KernelQuery();

  /**
   * @brief Get clocks from the PL
   *
   * This allows to check the current clocks from the PL in MHz.
   * This method is optionally implementable. If it is not implemented,
   * the number of elements of the vector is equal to zero.
   *
   * @returns a vector with a number of elements equal to the valid clocks
   */
  std::vector<float> GetClocks() noexcept override;
  /**
   * @brief Set clocks to the PL
   *
   * This allows to set the current clocks from the PL in MHz.
   * This method is optionally implementable. If it is not implemented,
   * no changes are performed
   *
   * @returns Status of the operation
   */
  Status SetClocks(const std::vector<float> &clocks) override;

 private:
  /** Parameters used for internal hardware configuration */
  std::shared_ptr<HardwareParameters> parameters_;
  /**
   * @brief Loads the bitstream
   *
   * Implementation-depend method to write the bitstream on the FPGA.
   * In this case, it uses the FPGA manager under the hood.
   *
   * @param bitstream_file path to the bitstream
   * @return Status
   */
  Status LoadBitstream(const std::string &bitstream_file);
  /**
   * @brief Configure the master and slave buses
   *
   * Configures the widths of the buses, enables them and configures
   * accesses.
   *
   * @return Status
   */
  Status ConfigureBuses();
  /**
   * @brief Retrieves the information from the current clocks
   *
   * This retrieves the system bus and the PLL clocks if they are enabled
   * and reports them back
   *
   * @param number_pl_clocks number of active PL clocks (from design). Def: 1
   */
  Status GetClocksInformation(const uint number_pl_clocks = 1);
  /**
   * @brief Configures the clocks according to the design
   *
   * The clock values are encapsulated in the hardware params. They can
   * be adjusted through an API as well as getting the information from
   * them.
   */
  Status ConfigureClocks();
  /**
   * @brief Loads the XCL Bin.
   *
   * In the case of the US+, it can be the default design if the bitstream
   * is provided or a custom default design with overlays and kernels
   *
   * @param xclbin_file path to the xclbin file
   * @param device_idx FPGA device index
   * @return Status
   */
  Status LoadXclBin(const std::string &xclbin_file, const int device_idx = 0);
};
}  // namespace cynq
