/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */

#include <xrt/xrt.h>
#include <xrt/xrt/xrt_bo.h>
#include <xrt/xrt/xrt_device.h>

#include <cynq/accelerator.hpp>
#include <cynq/dma/datamover.hpp>
#include <cynq/enums.hpp>
#include <cynq/hardware.hpp>
#include <cynq/mmio/accelerator.hpp>
#include <cynq/status.hpp>
#include <cynq/ultrascale/hardware.hpp>
#include <memory>
#include <stdexcept>
#include <string>

extern "C" {
#include <pynq_api.h> /* FIXME: to be removed in future releases */
}

#define CHECK_MMIO(val)                                       \
  {                                                           \
    int e = val;                                              \
    if (e != PYNQ_SUCCESS) {                                  \
      std::string msg = "Error while checking MMIO in line "; \
      msg += __func__;                                        \
      msg += ": ";                                            \
      msg += __LINE__;                                        \
      return Status{Status::CONFIGURATION_ERROR, msg};        \
    }                                                         \
  }

namespace cynq {
UltraScale::UltraScale(const std::string &bitstream_file,
                       const std::string &xclbin_file)
    : parameters_{std::make_shared<UltraScaleParameters>()} {
  /* For the UltraScale, there is only a single device. It is possible to
     load either a bitstream or a xclbin. */
  Status st{};

  /* Initial check: we want to make sure that both parameters are OK */
  if (xclbin_file.empty()) {
    throw std::runtime_error("Cannot work with an empty XCLBIN file");
  }

  /* Load the bitstream: the exception must propagate upwards */
  if (!bitstream_file.empty()) {
    st = LoadBitstream(bitstream_file);
    if (st.code != Status::OK) {
      std::string msg = "Error while loading the bitstream: ";
      msg += st.msg;
      throw std::runtime_error(msg);
    }
  }

  /* Configure the buses accordingly to the default design */
  st = ConfigureBuses();
  if (st.code != Status::OK) {
    std::string msg = "Error while configuring the buses: ";
    msg += st.msg;
    throw std::runtime_error(msg);
  }

  /* Configure the buses accordingly to the default design */
  st = LoadXclBin(xclbin_file);
  if (st.code != Status::OK) {
    std::string msg = "Error while configuring the buses: ";
    msg += st.msg;
    throw std::runtime_error(msg);
  }
}

Status UltraScale::LoadBitstream(const std::string &bitstream_file) {
  /* FIXME: This is a temporal implementation while we are coding our own
     implementation. Use with caution */
  auto res = PYNQ_loadBitstream(const_cast<char *>(bitstream_file.c_str()));
  std::string msg = "Cannot load the bitstream in location: ";
  msg += bitstream_file;
  return res == PYNQ_SUCCESS ? Status{} : Status{Status::FILE_ERROR, msg};
}

Status UltraScale::ConfigureBuses() {
  /* These addresses are hardware-specific. We have grabbed them from the
     original PYNQ project */
  static constexpr uint64_t addrs_sclr_kria[] = {0xFD615000, 0xFD615000,
                                                 0xFF419000};
  static constexpr uint8_t lowbitfields_sclr_kria[] = {8, 10, 8};
  static constexpr uint8_t maxigp_widths_kria[] = {2, 2,
                                                   0};  // 128, 128 and 32bits
  static constexpr uint64_t addrs_afifm_kria[] = {
      0xFD360000, 0xFD360014, 0xFD370000, 0xFD370014, 0xFD380000,
      0xFD380014, 0xFD390000, 0xFD390014, 0xFD3A0000, 0xFD3A0014,
      0xFD3B0000, 0xFD3B0014, 0xFF9B0000, 0xFF9B0014};
  const uint8_t lowbitfields_afifm_kria[] = {0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0};
  const uint8_t saxigp_widths_kria[] = {0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0};  // 128 all

  /* Write to the device memory window to configure them: master ifaces */
  for (int i = 0; i < 3; ++i) {
    uint32_t rval = 0, wval = 0, mask = 0b11;
    const int width = 4;  // 4 bytes

    PYNQ_MMIO_WINDOW win;
    CHECK_MMIO(PYNQ_createMMIOWindow(&win, addrs_sclr_kria[i], width));
    CHECK_MMIO(PYNQ_readMMIO(&win, &rval, 0x0, width));
    /* Set value */
    mask = mask << lowbitfields_sclr_kria[i];
    wval = rval;
    wval =
        (wval & ~mask) | (maxigp_widths_kria[i] << lowbitfields_sclr_kria[i]);
    /* Write value */
    CHECK_MMIO(PYNQ_writeMMIO(&win, &wval, 0x0, width));
    CHECK_MMIO(PYNQ_closeMMIOWindow(&win));
  }

  /* Write to the device memory window to configure them: slave ifaces */
  for (int i = 0; i < (7 * 2); ++i) {
    uint32_t rval = 0, wval = 0, mask = 0b11;
    const int width = 4;  // 4 bytes

    PYNQ_MMIO_WINDOW win;
    CHECK_MMIO(PYNQ_createMMIOWindow(&win, addrs_afifm_kria[i], width));
    CHECK_MMIO(PYNQ_readMMIO(&win, &rval, 0x0, width));
    /* Set value */
    mask = mask << lowbitfields_afifm_kria[i];
    wval = rval;
    wval =
        (wval & ~mask) | (saxigp_widths_kria[i] << lowbitfields_afifm_kria[i]);
    /* Write value */
    CHECK_MMIO(PYNQ_writeMMIO(&win, &wval, 0x0, width));
    CHECK_MMIO(PYNQ_closeMMIOWindow(&win));
  }

  return Status{};
}

Status UltraScale::LoadXclBin(const std::string &xclbin_file,
                              const int device_idx) {
  UltraScaleParameters *params =
      dynamic_cast<UltraScaleParameters *>(this->parameters_.get());
  if (!params) {
    return Status{Status::INCOMPATIBLE_PARAMETER,
                  "Hardware params incompatible"};
  }

  params->device_ = xrt::device(device_idx);
  params->device_.load_xclbin(xclbin_file);
  params->xclbin_ = xrt::xclbin(xclbin_file);

  return Status{};
}

Status UltraScale::Reset() { return Status{}; }

std::shared_ptr<IDataMover> UltraScale::GetDataMover(const uint64_t address) {
  return IDataMover::Create(IDataMover::DMA, address, this->parameters_);
}

std::shared_ptr<IAccelerator> UltraScale::GetAccelerator(
    const uint64_t address) {
  return IAccelerator::Create(IAccelerator::MMIO, address);
}

UltraScale::~UltraScale() {}

}  // namespace cynq
