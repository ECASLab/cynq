/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */

#include <cynq/ultrascale/hardware.hpp>

#include <memory>
#include <stdexcept>
#include <string>

#include <xrt/xrt.h>
#include <xrt/xrt/xrt_bo.h>
#include <xrt/xrt/xrt_device.h>

#include <cynq/accelerator.hpp>
#include <cynq/amd/accelerator.hpp>
#include <cynq/enums.hpp>
#include <cynq/hardware.hpp>
#include <cynq/status.hpp>
#include <cynq/xrt/datamover.hpp>

namespace cynq {

/**
 * @brief Specialisation of the parameters given by the UltraScale. This is
 * only available by the source file to encapsulate the dependencies involved.
 */
struct UltraScaleParameters : public HardwareParameters {
  /** XRT Device linked to the FPGA */
  xrt::device device_;
  /** XRT UUID that matches the device with the loaded XCLBIN */
  xrt::uuid uuid_;
  /** XRT class representing the xclbin object */
  xrt::xclbin xclbin_;
};

UltraScale::UltraScale(const std::string &bitstream_file,
                       const std::string &xclbin_file)
    : parameters_{std::make_unique<UltraScaleParameters>()} {
  /* For the UltraScale, there is only a single device. It is possible to
     load either a bitstream or a xclbin. */

  /* Initial check: we want to make sure that both parameters are OK */
  if (xclbin_file.empty()) {
    throw std::runtime_error("Cannot work with an empty XCLBIN file");
  }

  /* Load the bitstream: the exception must propagate upwards */
  if (!bitstream_file.empty()) {
    Status st = LoadBitstream(bitstream_file);
    if (st.code != Status::OK) {
      std::string msg = "Error while loading the bitstream: ";
      msg += st.msg;
      throw std::runtime_error(msg);
    }
  }

  /* Configure the buses accordingly to the default design */
  Status st = ConfigureBuses();
  if (st.code != Status::OK) {
    std::string msg = "Error while configuring the buses: ";
    msg += st.msg;
    throw std::runtime_error(msg);
  }

  /* Configure the buses accordingly to the default design */
  Status st = LoadXclBin(xclbin_file);
  if (st.code != Status::OK) {
    std::string msg = "Error while configuring the buses: ";
    msg += st.msg;
    throw std::runtime_error(msg);
  }
}

Status UltraScale::LoadBitstream(const std::string &bitstream_file) {
  return Status{};
}

Status UltraScale::ConfigureBuses() { return Status{}; }

Status UltraScale::LoadXclBin(const std::string &xclbin_file,
                              const int device_idx = 0) {
  return Status{};
}

Status UltraScale::Reset() { return Status{}; }

std::shared_ptr<IDataMover> UltraScale::GetDataMover(const uint64_t address) {
  return IDataMover::Create(IDataMover::XRT, address);
}

std::shared_ptr<IAccelerator> UltraScale::GetAccelerator(
    const uint64_t address) {
  return IAccelerator::Create(IAccelerator::XRT, address);
}
}  // namespace cynq
