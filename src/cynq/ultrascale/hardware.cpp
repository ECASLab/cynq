/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <xrt.h>
#include <xrt/xrt_bo.h>
#include <xrt/xrt_device.h>
#pragma GCC diagnostic pop

#define DEBUG_MODE 5

#include <cynq/accelerator.hpp>
#include <cynq/debug.hpp>
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
      msg += std::string(__func__);                           \
      msg += ": ";                                            \
      msg += std::to_string(__LINE__);                        \
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

  GetClocksInformation();
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

template <typename T>
static T GetSlice(const T input, const uint end, const uint start) {
  T mask = 1 << (end - start);
  mask -= 1;
  T res = input;
  return (res >> start) & mask;
}

template <typename T>
static T GetField(const T input, const uint start) {
  T mask = 0x1;
  T res = input;
  return (res >> start) & mask;
}

Status UltraScale::GetClocksInformation(const uint number_pl_clocks) {
  /* This information comes from the PYNQ code according to a default
     design without major modifications

     Slices start from 0 index and the end are exclusive bounds
     (not included) */
  /* APB address and offsets */
  static constexpr uint crl_apb_address = 0xFF5E0000;
  static constexpr uint max_number_pl_clocks = 4;
  static constexpr uint pl_ctrl_offsets[] = {0xC0, 0xC4, 0xC8, 0xCC};
  static constexpr uint pl_src_pll_ctrls[] = {0x20, 0x20, 0x30, 0x2C};
  /* Source clock */
  static constexpr uint plx_ctrl_clkact_field_bitfield = 24;
  static constexpr uint crx_apb_src_default = 0;
  static constexpr uint crx_apb_src_field_start = 20;
  static constexpr uint crx_apb_src_field_end = 22;
  static constexpr uint crx_apb_fbdiv_field_start = 8;
  static constexpr uint crx_apb_fbdiv_field_end = 14;
  static constexpr uint crx_apb_odivby2_bitfield = 16;
  static const float default_src_clock_mhz = 33.333;
  /* PLL div */
  static constexpr uint pl_clk_odiv0_field_start = 16;
  static constexpr uint pl_clk_odiv0_field_end = 21;
  static constexpr uint pl_clk_odiv1_field_start = 8;
  static constexpr uint pl_clk_odiv1_field_end = 13;

  PYNQ_MMIO_WINDOW crl_apb_win;
  const int crl_apb_width = 0x100;
  CHECK_MMIO(
      PYNQ_createMMIOWindow(&crl_apb_win, crl_apb_address, crl_apb_width));

  std::array<bool, max_number_pl_clocks> pl_active;
  std::array<bool, max_number_pl_clocks> pl_valid;
  std::array<float, max_number_pl_clocks> src_freq;
  std::array<float, max_number_pl_clocks> pl_freq;
  std::array<uint32_t, max_number_pl_clocks> pl_reg;
  std::array<uint32_t, max_number_pl_clocks> src_reg;

  for (uint i = 0; i < number_pl_clocks; ++i) {
    CHECK_MMIO(PYNQ_readMMIO(&crl_apb_win, &pl_reg[i], pl_ctrl_offsets[i],
                             sizeof(uint32_t)));
    CHECK_MMIO(PYNQ_readMMIO(&crl_apb_win, &src_reg[i], pl_src_pll_ctrls[i],
                             sizeof(uint32_t)));
    /* Check if it's active */
    pl_active[i] = GetField(pl_reg[i], plx_ctrl_clkact_field_bitfield);
    /* Check if it's valid */
    uint apb_src_field =
        GetSlice(src_reg[i], crx_apb_src_field_end, crx_apb_src_field_start);
    pl_valid[i] = apb_src_field == crx_apb_src_default;
    /* If not valid */
    if (!pl_valid[i]) continue;
    /* Compute source frequency */
    float fbdiv = static_cast<float>(GetSlice(
        src_reg[i], crx_apb_fbdiv_field_end, crx_apb_fbdiv_field_start));
    float div2 =
        GetField(src_reg[i], crx_apb_odivby2_bitfield) == 0x1 ? 0.5f : 1.f;
    src_freq[i] = default_src_clock_mhz * fbdiv * div2;
    /* Compute PL clock */
    float plldiv0 =
        1.f / static_cast<float>(GetSlice(pl_reg[i], pl_clk_odiv0_field_end,
                                          pl_clk_odiv0_field_start));
    float plldiv1 =
        1.f / static_cast<float>(GetSlice(pl_reg[i], pl_clk_odiv1_field_end,
                                          pl_clk_odiv1_field_start));
    pl_freq[i] = src_freq[i] * plldiv0 * plldiv1;

    CYNQ_DEBUG(LOG::DEBUG, "Active: ", pl_active[i]);
    CYNQ_DEBUG(LOG::DEBUG, "Active: ", pl_active[i]);
    CYNQ_DEBUG(LOG::DEBUG, "Active:", pl_active[i]);
    CYNQ_DEBUG(LOG::DEBUG, "Valid:", pl_valid[i]);
    CYNQ_DEBUG(LOG::DEBUG, "FbDiv:", fbdiv);
    CYNQ_DEBUG(LOG::DEBUG, "Div2:", div2);
    CYNQ_DEBUG(LOG::DEBUG, "SRC freq:", src_freq[i], " MHz");
    CYNQ_DEBUG(LOG::DEBUG, "PL Div0:", plldiv0, "PL Div1:", plldiv1);
    CYNQ_DEBUG(LOG::DEBUG, "PL freq:", pl_freq[i], " MHz");
  }

  // std::cout << "Mask: 5, 2 " << (uint32_t)GetSlice<uint>(0b1111010, 5, 2) <<
  // std::endl;

  CHECK_MMIO(PYNQ_closeMMIOWindow(&crl_apb_win));
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

std::shared_ptr<IAccelerator> UltraScale::GetAccelerator(
    const std::string & /* kernelname */) {
  return nullptr;
}

UltraScale::~UltraScale() {}

}  // namespace cynq
