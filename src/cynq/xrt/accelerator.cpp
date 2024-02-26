/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include <xrt/xrt.h>
#include <xrt/xrt/xrt_kernel.h>
#include <xrt/xrt/xrt_uuid.h>

#include <cynq/accelerator.hpp>
#include <cynq/alveo/hardware.hpp>
#include <cynq/enums.hpp>
#include <cynq/status.hpp>
#include <cynq/xrt/accelerator.hpp>
#include <memory>
#include <stdexcept>
#include <string>

namespace cynq {
/**
 * @brief Specialisation of the parameters given by the XRT. This is
 * only available by the source file to encapsulate the dependencies involved.
 */
struct XRTAcceleratorParameters : public AcceleratorParameters {
  /** Kernel object */
  xrt::kernel kernel_;
  /** Hardware parameters */
  std::weak_ptr<HardwareParameters> hwparams_;
  /** Kernel run wrapper */
  xrt::run run_;
  /** Virtual destructor required for the inheritance */
  virtual ~XRTAcceleratorParameters() = default;
};

XRTAccelerator::XRTAccelerator(
    const std::string &kernelname,
    const std::shared_ptr<HardwareParameters> hwparams)
    : accel_params_{std::make_unique<XRTAcceleratorParameters>()} {
  /* The assumption is that at this point, it is ok */
  auto params = dynamic_cast<XRTAcceleratorParameters *>(accel_params_.get());

  /* Cast the params to be compatible with Alveo
     This needs to be changed later */
  auto xrthwparams = std::dynamic_pointer_cast<AlveoParameters>(hwparams);
  if (!xrthwparams) {
    throw Status{Status::INCOMPATIBLE_PARAMETER,
                 "The parameters do not match to the Alveo Parameters"};
  }
  /* Create the XRT Kernel */
  params->hwparams_ = xrthwparams;
  params->kernel_ =
      xrt::kernel(xrthwparams->device_, xrthwparams->uuid_, kernelname);
  params->run_ = xrt::run(params->kernel_);

  if (!params->run_) {
    throw Status{Status::CONFIGURATION_ERROR,
                 "Cannot create the xrt::run instance"};
  }
}

Status XRTAccelerator::Start(const StartMode mode) {
  if (StartMode::Continuous == mode) {
    return Status{Status::NOT_IMPLEMENTED, "Not implemented"};
  } else {
    auto params = dynamic_cast<XRTAcceleratorParameters *>(accel_params_.get());
    params->run_.start();
  }
  return Status{};
}

Status XRTAccelerator::Stop() {
  auto params = dynamic_cast<XRTAcceleratorParameters *>(accel_params_.get());
  params->run_.stop();
  return Status{};
}

Status XRTAccelerator::Sync() {
  auto params = dynamic_cast<XRTAcceleratorParameters *>(accel_params_.get());
  ert_cmd_state ert = params->run_.wait();
  if (ert != ERT_CMD_STATE_COMPLETED) {
    return Status{Status::EXECUTION_FAILED, "Error: " + std::to_string(ert)};
  }
  return Status{};
}

DeviceStatus XRTAccelerator::GetStatus() {
  auto params = dynamic_cast<XRTAcceleratorParameters *>(accel_params_.get());
  ert_cmd_state ert = params->run_.state();
  switch (ert) {
    case ERT_CMD_STATE_RUNNING:
      return DeviceStatus::Running;
    case ERT_CMD_STATE_COMPLETED:
      return DeviceStatus::Done;
    case ERT_CMD_STATE_NEW:
      [[fallthrough]];
    case ERT_CMD_STATE_QUEUED:
      return DeviceStatus::Idle;
    default:
      return DeviceStatus::Unknown;
  }
}

Status XRTAccelerator::WriteRegister(const uint64_t address,
                                     const uint8_t *data, const size_t size) {
  auto params = dynamic_cast<XRTAcceleratorParameters *>(accel_params_.get());

  /* Check alignment */
  if (size % 4 != 0) {
    return Status{Status::REGISTER_NOT_ALIGNED,
                  "The size must be aligned to 32 bits"};
  }

  /* Write the register */
  try {
    const size_t sizeu32 = size >> 2;
    const uint32_t *datau32 = reinterpret_cast<const uint32_t *>(data);

    /* Get the offset of the reg */
    uint32_t offset = params->kernel_.offset(address);

    for (uint i = 0; i < sizeu32; ++i) {
      params->kernel_.write_register(offset, datau32[i]);
    }
  } catch (std::exception &e) {
    return Status{Status::REGISTER_IO_ERROR,
                  std::string("Cannot write to the register - ") + e.what()};
  }
  return Status{};
}

Status XRTAccelerator::ReadRegister(const uint64_t address, uint8_t *data,
                                    const size_t size) {
  auto params = dynamic_cast<XRTAcceleratorParameters *>(accel_params_.get());

  /* Check alignment */
  if (size % 4 != 0) {
    return Status{Status::REGISTER_NOT_ALIGNED,
                  "The size must be aligned to 32 bits"};
  }

  /* Write the register */
  try {
    const size_t sizeu32 = size >> 2;
    uint32_t *datau32 = reinterpret_cast<uint32_t *>(data);

    /* Get the offset of the reg */
    uint32_t offset = params->kernel_.offset(address);

    for (uint i = 0; i < sizeu32; ++i) {
      datau32[i] = params->kernel_.read_register(offset);
    }
  } catch (std::exception &e) {
    return Status{Status::REGISTER_IO_ERROR,
                  std::string("Cannot read from the register - ") + e.what()};
  }
  return Status{};
}

Status XRTAccelerator::AttachRegister(const uint64_t index, uint8_t *data,
                                      const size_t size) {
  auto params = dynamic_cast<XRTAcceleratorParameters *>(accel_params_.get());
  if (index > 255 || !data || size == 0) {
    return Status{Status::INVALID_PARAMETER,
                  "index and size must be greater than 0. data must be valid"};
  }
  params->run_.set_arg((int)index, (const void *)data, size);  // NOLINT
  return Status{};
}

XRTAccelerator::~XRTAccelerator() {}

int XRTAccelerator::GetMemoryBank(const uint pos) {
  auto params = dynamic_cast<XRTAcceleratorParameters *>(accel_params_.get());
  return params->kernel_.group_id(pos);
}

}  // namespace cynq
