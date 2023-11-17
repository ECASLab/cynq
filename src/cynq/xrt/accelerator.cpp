/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include <memory>
#include <stdexcept>
#include <string>

#include <cynq/xrt/accelerator.hpp>

#include <cynq/accelerator.hpp>
#include <cynq/enums.hpp>
#include <cynq/status.hpp>

extern "C" {
#include <pynq_api.h> /* FIXME: to be removed in future releases */
}

/*
 * FIXME: This implementation is fully based on the PYNQ C API from the
 * community. This must be updated once the PYNQ C port as been done
 * in a decent manner.
 */

static constexpr uint64_t kAddrSpace = 65536;

namespace cynq {
/**
 * @brief Specialisation of the parameters given by the UltraScale. This is
 * only available by the source file to encapsulate the dependencies involved.
 */
struct XrtAcceleratorParameters : public AcceleratorParameters {
  /** Accelerator address */
  uint64_t addr_;
  /** Address space size */
  uint64_t addr_space_size_;
  /** HLS Design */
  PYNQ_HLS hls_;
  /** Virtual destructor required for the inheritance */
  virtual ~XrtAcceleratorParameters() = default;
};

XRTAccelerator::XRTAccelerator(const uint64_t addr)
    : addr_{addr},
      addr_space_size_{kAddrSpace},
      accel_params_{std::make_unique<XrtAcceleratorParameters>()} {
  /* The assumption is that at this point, it is ok */
  auto params = dynamic_cast<XrtAcceleratorParameters *>(accel_params_.get());

  params->addr_ = this->addr_;
  params->addr_space_size_ = this->addr_space_size_;

  if (PYNQ_SUCCESS !=
      PYNQ_openHLS(&params->hls_, this->addr_, this->addr_space_size_)) {
    std::string msg = "Cannot open the design in addr: ";
    msg += std::to_string(this->addr_);
    throw std::runtime_error(msg);
  }
}

Status XRTAccelerator::Start(const StartMode mode) {
  constexpr uint64_t ctrl_reg_addr = 0x00;
  const uint8_t ctrl_reg_val = StartMode::Once == mode ? 0x01 : 0x81;
  return this->WriteRegister(ctrl_reg_addr, &ctrl_reg_val, sizeof(uint8_t));
}

Status XRTAccelerator::Stop() {
  constexpr uint64_t ctrl_reg_addr = 0x00;
  const uint8_t ctrl_reg_val = 0x0;
  return this->WriteRegister(ctrl_reg_addr, &ctrl_reg_val, sizeof(uint8_t));
}

DeviceStatus XRTAccelerator::GetStatus() {
  constexpr uint64_t ctrl_reg_addr = 0x00;
  uint8_t ctrl_reg_val = 0x0;

  Status st = this->ReadRegister(ctrl_reg_addr, &ctrl_reg_val, sizeof(uint8_t));
  if (Status::OK != st.code) {
    return DeviceStatus::Error;
  }

  switch (ctrl_reg_val) {
    case 0x01:
    case 0x03:
    case 0x81:
    case 0x83:
      return DeviceStatus::Running;
    case 0x04:
      return DeviceStatus::Idle;
    case 0x06:
      return DeviceStatus::Done;
    default:
      return DeviceStatus::Unknown;
  }
}

Status XRTAccelerator::WriteRegister(const uint64_t address,
                                     const uint8_t *data, const size_t size) {
  auto params = dynamic_cast<XrtAcceleratorParameters *>(accel_params_.get());
  auto ret = PYNQ_writeToHLS(&params->hls_, const_cast<uint8_t *>(data),
                             address, size);
  if (PYNQ_SUCCESS != ret) {
    std::string msg = "Cannot write on HLS register: ";
    msg += std::to_string(address);
    msg += " the payload with size: ";
    msg += std::to_string(size);
    return Status{Status::REGISTER_IO_ERROR, msg};
  }
  return Status{};
}

Status XRTAccelerator::ReadRegister(const uint64_t address, uint8_t *data,
                                    const size_t size) {
  auto params = dynamic_cast<XrtAcceleratorParameters *>(accel_params_.get());
  auto ret = PYNQ_readFromHLS(&params->hls_, data, address, size);
  if (PYNQ_SUCCESS != ret) {
    std::string msg = "Cannot read on HLS register: ";
    msg += std::to_string(address);
    msg += " the payload with size: ";
    msg += std::to_string(size);
    return Status{Status::REGISTER_IO_ERROR, msg};
  }
  return Status{};
}

XRTAccelerator::~XRTAccelerator() {
  /* The assumption is that at this point, it is ok */
  auto params = dynamic_cast<XrtAcceleratorParameters *>(accel_params_.get());
  PYNQ_closeHLS(&params->hls_);
}

}  // namespace cynq
