/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2025
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include <cynq/hardware.hpp>
#include <cynq/mmio/axi-gpio.hpp>
#include <iostream>
#include <memory>

static constexpr uint32_t HP0_DDR_LOW = 0x0;
static constexpr uint32_t HP0_DDR_HIGH = 0x80000000;
static constexpr uint32_t HP0_QSPI = 0xC0000000;
static constexpr uint32_t AXI_CONTROL_REG = 2684878848;

#define XRV32I_NPP_IP_CONTROL_ADDR_AP_CTRL 0x00
#define XRV32I_NPP_IP_CONTROL_ADDR_GIE 0x04
#define XRV32I_NPP_IP_CONTROL_ADDR_IER 0x08
#define XRV32I_NPP_IP_CONTROL_ADDR_ISR 0x0c
#define XRV32I_NPP_IP_CONTROL_ADDR_START_PC_DATA 0x10
#define XRV32I_NPP_IP_CONTROL_BITS_START_PC_DATA 32
#define XRV32I_NPP_IP_CONTROL_ADDR_NB_INSTRUCTION_DATA 0x18
#define XRV32I_NPP_IP_CONTROL_BITS_NB_INSTRUCTION_DATA 32
#define XRV32I_NPP_IP_CONTROL_ADDR_NB_INSTRUCTION_CTRL 0x1c

static constexpr char kBitstream[] = "examples/zynq-mpsoc/rvv_ip.bit";

int main() {
  using namespace cynq;  // NOLINT

  std::cout << "Platform initialization \n";

  std::shared_ptr<IHardware> platform = IHardware::Create(
      HardwareArchitecture::UltraScale, kBitstream);  // NOLINT
  auto axiControlReg = platform->GetAccelerator(AXI_CONTROL_REG);
  auto QSPI = platform->GetAccelerator(HP0_QSPI);

  return 0;
}
