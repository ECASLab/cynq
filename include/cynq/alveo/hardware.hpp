/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */
#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <xrt.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_uuid.h>
#pragma GCC diagnostic pop

#include <cynq/accelerator.hpp>
#include <cynq/datamover.hpp>
#include <cynq/enums.hpp>
#include <cynq/hardware.hpp>
#include <cynq/status.hpp>
#include <memory>
#include <string>

namespace cynq {
/**
 * @brief Specialisation of the parameters given by the Alveo. It
 * is based on the XRT stack
 */
struct AlveoParameters : public HardwareParameters {
  /** XRT Device linked to the FPGA */
  xrt::device device_;
  /** XRT class representing the xclbin object */
  xrt::xclbin xclbin_;
  /** XRT class corresponding to the UUID of the xclbin */
  xrt::uuid uuid_;
  /** XCLBIN file path */
  std::string xclbin_file_;

  /** Virtual destructor required for the inheritance */
  virtual ~AlveoParameters() = default;
};

/**
 * @brief Alveo class
 * Provides an interface to access IP Cores in Xilinx FPGAs, the compatible
 * devices are the Xilinx Alveo
 *
 * This class does not have support for bitstream
 */
class Alveo : public IHardware {
 public:
  /**
   * @brief Construct a new Alveo object
   *
   * Configure the FPGA with an xclbin object. The bitstream is unused and this
   * only takes into account the xclbin file.
   *
   * As future work, the XCLBIN will be embedded into the binary to avoid
   * reading it from an external file.
   *
   * @param bitstream_file "unused"
   * @param xclbin_file full path to the xclbin object.
   */
  Alveo(const std::string &bitstream_file, const std::string &xclbin_file);
  /**
   * No default constructor required
   */
  Alveo() = delete;
  /**
   * @brief Alveo destructor method
   * Destroy the Alveo object.
   */
  virtual ~Alveo();
  /**
   * @brief Reset method
   * Sets the Alveo instance to its initial state.
   *
   * @return Status
   */
  Status Reset() override;
  /**
   * @brief GetDataMover method
   * Used for accessing the IDataMover instance of the Alveo object. It
   * allocates buffers into an specific memory bank inside of the Alveo
   * card
   *
   * @param address it is the memory bank id. You can get the memory bank
   * by querying the cynq::IAccelerator::GetMemoryBank.
   *
   * @return std::shared_ptr<IDataMover>
   * Returns an IDataMover pointer with reference counting. It is not
   * MT-safe.
   *
   */
  std::shared_ptr<IDataMover> GetDataMover(const uint64_t address) override;
  /**
   * @brief GetAccelerator method
   *
   * Finds and wraps the XRT Kernel into an IAccelerator object to make the
   * API uniform across platforms. There are still differences between the
   * Alveo cards and Ultrascale that will be addressed in the future.
   *
   * @param kernelname kernel name for XRT kernel
   *
   * @return std::shared_ptr<IAccelerator> nullptr
   *
   */
  std::shared_ptr<IAccelerator> GetAccelerator(
      const std::string &kernelname) override;

  /**
   * @brief GetAccelerator method (overload - not implemented)
   *
   * Do not use this method since it is not implemented and it will lead
   * to a nullptr
   *
   * @param address a unsigned integer of 64 bits representing an address.
   *
   * @return std::shared_ptr<IAccelerator> nullptr
   *
   */
  std::shared_ptr<IAccelerator> GetAccelerator(const uint64_t address) override;

 private:
  /** Parameters used for internal hardware configuration */
  std::shared_ptr<HardwareParameters> parameters_;

  /**
   * @brief Loads the XCL Bin.
   *
   * @param xclbin_file path to the xclbin file
   * @param device_idx FPGA device index
   * @return Status
   */
  Status LoadXclBin(const std::string &xclbin_file, const int device_idx = 0);
};
}  // namespace cynq
