/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include <cynq/accelerator.hpp>
#include <cynq/enums.hpp>
#include <cynq/mmio/accelerator.hpp>
#include <cynq/status.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>

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
struct MMIOAcceleratorParameters : public AcceleratorParameters {
  /** Accelerator address */
  uint64_t addr_;
  /** Address space size */
  uint64_t addr_space_size_;
  /** HLS Design */
  PYNQ_HLS hls_;
  /**
   * Map with the arguments attached to it with synchronisation purposes. The
   * first argument is the address, the pair argument is a composition of the
   * pointer to write on/from (requires 32-bit alignment), the register
   * access kind and the size (aligned to 32-bit)
   */
  std::unordered_map<uint64_t, std::tuple<uint8_t *, RegisterAccess, size_t>>
      accel_attachments_;
  /** Virtual destructor required for the inheritance */
  virtual ~MMIOAcceleratorParameters() = default;
};

MMIOAccelerator::MMIOAccelerator(const uint64_t addr)
    : addr_{addr},
      addr_space_size_{kAddrSpace},
      accel_params_{std::make_unique<MMIOAcceleratorParameters>()} {
  /* The assumption is that at this point, it is ok */
  auto params = dynamic_cast<MMIOAcceleratorParameters *>(accel_params_.get());

  params->addr_ = this->addr_;
  params->addr_space_size_ = this->addr_space_size_;

  if (PYNQ_SUCCESS !=
      PYNQ_openHLS(&params->hls_, this->addr_, this->addr_space_size_)) {
    std::string msg = "Cannot open the design in addr: ";
    msg += std::to_string(this->addr_);
    throw std::runtime_error(msg);
  }
}

Status MMIOAccelerator::Start(const StartMode mode) {
  Status ret{};
  constexpr uint64_t ctrl_reg_addr = 0x00;
  const uint8_t ctrl_reg_val = StartMode::Once == mode ? 0x01 : 0x81;
  ret = this->SyncRegisters(SyncType::HostToDevice);
  if (ret.code) return ret;
  return this->WriteRegister(ctrl_reg_addr, &ctrl_reg_val, sizeof(uint8_t));
}

Status MMIOAccelerator::Stop() {
  Status ret{};
  constexpr uint64_t ctrl_reg_addr = 0x00;
  const uint8_t ctrl_reg_val = 0x0;
  ret = this->SyncRegisters(SyncType::DeviceToHost);
  if (ret.code) return ret;
  return this->WriteRegister(ctrl_reg_addr, &ctrl_reg_val, sizeof(uint8_t));
}

Status MMIOAccelerator::Sync() {
  Status ret{};
  while (DeviceStatus::Running == this->GetStatus()) {
  }
  ret = this->SyncRegisters(SyncType::DeviceToHost);
  return ret;
}

DeviceStatus MMIOAccelerator::GetStatus() {
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

Status MMIOAccelerator::WriteRegister(const uint64_t address,
                                      const uint8_t *data, const size_t size) {
  auto params = dynamic_cast<MMIOAcceleratorParameters *>(accel_params_.get());
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

Status MMIOAccelerator::ReadRegister(const uint64_t address, uint8_t *data,
                                     const size_t size) {
  auto params = dynamic_cast<MMIOAcceleratorParameters *>(accel_params_.get());
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

Status MMIOAccelerator::SyncRegisters(const SyncType type) {
  Status status{};
  auto params = dynamic_cast<MMIOAcceleratorParameters *>(accel_params_.get());

  auto it = params->accel_attachments_.begin();
  auto endit = params->accel_attachments_.end();

  for (; it != endit; it++) {
    /* Get the key-val from the attachments */
    uint64_t reg_addr = it->first;
    auto reg_props = it->second;

    /* Decompose the props */
    uint8_t *ptr = std::get<0>(reg_props);
    RegisterAccess access = std::get<1>(reg_props);
    size_t size = std::get<2>(reg_props);

    if (SyncType::HostToDevice == type) {
      /* Handle the upload (write time) */
      if (access == RegisterAccess::RO) continue;
      status = this->WriteRegister(reg_addr, ptr, size);
    } else {
      /* Handle the download (read time) */
      if (access == RegisterAccess::WO) continue;
      status = this->ReadRegister(reg_addr, ptr, size);
    }

    if (Status::OK != status.code) break;
  }

  return status;
}

Status MMIOAccelerator::AttachRegister(const uint64_t index, uint8_t *data,
                                       const RegisterAccess access,
                                       const size_t size) {
  auto params = dynamic_cast<MMIOAcceleratorParameters *>(accel_params_.get());

  /* Delete the attachment */
  if (!data) {
    params->accel_attachments_.erase(index);
    return Status{};
  }

  /* Check alignment */
  if ((size & 0b11) != 0) {
    return Status{Status::INVALID_PARAMETER,
                  "The element size must be 4 bytes aligned"};
  }

  params->accel_attachments_[index] = {data, access, size};
  return Status{};
}

Status MMIOAccelerator::Attach(const uint64_t addr,
                               std::shared_ptr<IMemory> mem) {
  if (!mem) {
    return Status{Status::INVALID_PARAMETER, "The pointer is null"};
  }

  auto ptr = mem->DeviceAddress<uint8_t>().get();
  if (!ptr) {
    return Status{
        Status::INVALID_PARAMETER,
        "The device pointer is null. Are you passing a device-valid memory?"};
  }

  uint64_t addrps = reinterpret_cast<uint64_t>(ptr);
  uint32_t addrpl = addrps;

  return this->WriteRegister(addr, reinterpret_cast<uint8_t *>(&addrpl),
                             sizeof(decltype(addrpl)));
}

MMIOAccelerator::~MMIOAccelerator() {
  /* The assumption is that at this point, it is ok */
  auto params = dynamic_cast<MMIOAcceleratorParameters *>(accel_params_.get());
  PYNQ_closeHLS(&params->hls_);
}

int MMIOAccelerator::GetMemoryBank(const uint /* pos */) { return 0; }

}  // namespace cynq
