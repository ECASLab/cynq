/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2025
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */

#include <pybind11/embed.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <cynq/accelerator.hpp>
#include <cynq/datamover.hpp>
#include <cynq/enums.hpp>
#include <cynq/hardware.hpp>
#include <cynq/memory.hpp>
#include <cynq/mmio/axi-gpio.hpp>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

// Given by the example
static constexpr char kBitstream[] = "examples/zynq-mpsoc/reckon/pynqKria.bit";

constexpr uint XSP_DGIER_OFFSET = 0x1C;
constexpr uint XSP_IISR_OFFSET = 0x20;
constexpr uint XSP_IIER_OFFSET = 0x28;
constexpr uint XSP_SRR_OFFSET = 0x40;
constexpr uint XSP_CR_OFFSET = 0x60;
constexpr uint XSP_SR_OFFSET = 0x64;
constexpr uint XSP_DTR_OFFSET = 0x68;
constexpr uint XSP_DRR_OFFSET = 0x6C;
constexpr uint XSP_SSR_OFFSET = 0x70;
constexpr uint XSP_TFO_OFFSET = 0x74;
constexpr uint XSP_RFO_OFFSET = 0x78;
constexpr uint XSP_REGISTERS[] = {0x40, 0x60, 0x64, 0x68, 0x6c, 0x70,
                                  0x74, 0x78, 0x1c, 0x20, 0x28};

constexpr uint XSP_SRR_RESET_MASK = 0x0A;
constexpr uint XSP_SR_TX_EMPTY_MASK = 0x04;
constexpr uint XSP_SR_TX_FULL_MASK = 0x08;
constexpr uint XSP_CR_TRANS_INHIBIT_MASK = 0x100;
constexpr uint XSP_CR_LOOPBACK_MASK = 0x01;
constexpr uint XSP_CR_ENABLE_MASK = 0x02;
constexpr uint XSP_CR_MASTER_MODE_MASK = 0x04;
constexpr uint XSP_CR_CLK_POLARITY_MASK = 0x08;
constexpr uint XSP_CR_CLK_PHASE_MASK = 0x10;
constexpr uint XSP_CR_TXFIFO_RESET_MASK = 0x20;
constexpr uint XSP_CR_RXFIFO_RESET_MASK = 0x40;
constexpr uint XSP_CR_MANUAL_SS_MASK = 0x80;

// PROGRAMMING SNN PARAMETERS
constexpr uint32_t SPI_EN_CONF = 0x00000001;
constexpr uint32_t SPI_DIS_CONF = 0x00000000;
constexpr uint32_t SPI_RO_STAGE_SEL = 0x00000000;
constexpr uint32_t SPI_GET_CLKINT_OUT = 0x00000000;
constexpr uint32_t SPI_RST_MODE = 0x00000000;
constexpr uint32_t SPI_DO_EPROP = 0x00000000;
constexpr uint32_t SPI_LOCAL_TICK = 0x00000000;
constexpr uint32_t SPI_ERROR_HALT = 0x00000001;
constexpr uint32_t SPI_FP_LOC_WINP = 0x00000003;
constexpr uint32_t SPI_FP_LOC_WREC = 0x00000003;
constexpr uint32_t SPI_FP_LOC_WOUT = 0x00000003;
constexpr uint32_t SPI_FP_LOC_TINP = 0x00000003;
constexpr uint32_t SPI_FP_LOC_TREC = 0x00000004;
constexpr uint32_t SPI_FP_LOC_TOUT = 0x00000006;
constexpr uint32_t SPI_LEARN_SIG_SCALE = 0x00000005;
constexpr uint32_t SPI_REGUL_MODE = 0x00000002;
constexpr uint32_t SPI_REGUL_W = 0x00000003;
constexpr uint32_t SPI_EN_STOCH_ROUND = 0x00000001;
constexpr uint32_t SPI_SRAM_SPEEDMODE = 0x00000000;
constexpr uint32_t SPI_TIMING_MODE = 0x00000000;
constexpr uint32_t SPI_REGRESSION = 0x00000000;
constexpr uint32_t SPI_SINGLE_LABEL = 0x00000001;
constexpr uint32_t SPI_NO_OUT_ACT = 0x00000000;
constexpr uint32_t SPI_SEND_PER_TIMESTEP = 0x00000000;
constexpr uint32_t SPI_SEND_LABEL_ONLY = 0x00000001;
constexpr uint32_t SPI_NOISE_EN = 0x00000000;
uint32_t SPI_FORCE_TRACES = 0x00000000;
constexpr uint32_t SPI_CYCLES_PER_TICK = 0x00000000;
constexpr uint32_t SPI_ALPHA_CONF0 = 0x00000000;
constexpr uint32_t SPI_ALPHA_CONF1 = 0x00000000;
constexpr uint32_t SPI_ALPHA_CONF2 = 0x00000000;
constexpr uint32_t SPI_ALPHA_CONF3 = 0x00000000;
constexpr uint32_t SPI_KAPPA = 0x00000079;
constexpr uint32_t SPI_THR_H_0 = 0x000000CD;
constexpr uint32_t SPI_THR_H_1 = 0x000000CD;
constexpr uint32_t SPI_THR_H_2 = 0x000000CD;
constexpr uint32_t SPI_THR_H_3 = 0x000000CD;
constexpr uint32_t SPI_H_0 = 0x00000000;
constexpr uint32_t SPI_H_1 = 0x00000000;
constexpr uint32_t SPI_H_2 = 0x00000000;
constexpr uint32_t SPI_H_3 = 0x00000000;
constexpr uint32_t SPI_H_4 = 0x00000001;
static uint32_t SPI_LR_R_WINP = 0x00000000;
static uint32_t SPI_LR_P_WINP = 0x00000008;
static uint32_t SPI_LR_R_WREC = 0x00000000;
static uint32_t SPI_LR_P_WREC = 0x00000008;
static uint32_t SPI_LR_R_WOUT = 0x00000000;
static uint32_t SPI_LR_P_WOUT = 0x0000000e;
constexpr uint32_t SPI_SEED_INP = 0x0000f0f0;
constexpr uint32_t SPI_SEED_REC = 0x0000f1f1;
constexpr uint32_t SPI_SEED_OUT = 0x0000f2f2;
constexpr uint32_t SPI_SEED_STRND_NEUR = 0x3f3ff3f3;
constexpr uint32_t SPI_SEED_STRND_ONEUR = 0x0000f4f4;
constexpr uint32_t SPI_SEED_STRND_TINP = 0x3f5ff5f5;
constexpr uint32_t SPI_SEED_STRND_TREC = 0x3f6ff6f6;
constexpr uint32_t SPI_SEED_STRND_TOUT = 0x3f7ff7f7;
constexpr uint32_t SPI_SEED_NOISE_NEUR = 0x00000ff0;
constexpr uint32_t SPI_NUM_INP_NEUR = 0x00000017;
constexpr uint32_t SPI_NUM_REC_NEUR = 0x000000C8;
constexpr uint32_t SPI_NUM_OUT_NEUR = 0x00000001;
constexpr uint32_t SPI_REGUL_F0 = 0x000000A0;
constexpr uint32_t SPI_REGUL_K_INP_R = 0x00000000;
constexpr uint32_t SPI_REGUL_K_INP_P = 0x0000000a;
constexpr uint32_t SPI_REGUL_K_REC_R = 0x00000000;
constexpr uint32_t SPI_REGUL_K_REC_P = 0x0000000a;
constexpr uint32_t SPI_REGUL_K_MUL = 0x00000000;
constexpr uint32_t SPI_NOISE_STR = 0x00000000;
constexpr uint64_t SLAVE_NO_SELECTION = 0xFFFFFFFF;
namespace py = pybind11;  // NOLINT

class EpochData {
 public:
  uint32_t number = 0;
  std::string name = "";
  double precision = 0.0;
  double sampleTimes = 0.0;
  double dmaTimes = 0.0;
};

typedef struct {
  uint32_t alpha;
  uint32_t firing;
  uint32_t lr_p_inp;
  uint32_t lr_p_rec;
  uint32_t lr_p_out;
  uint32_t epoch;
} experiment;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wpedantic"
experiment e7 = {.alpha = 3964,
                 .firing = 360,
                 .lr_p_inp = 13,
                 .lr_p_rec = 11,
                 .lr_p_out = 13,
                 .epoch = 10};
experiment e3 = {.alpha = 3824,
                 .firing = 631,
                 .lr_p_inp = 12,
                 .lr_p_rec = 12,
                 .lr_p_out = 12,
                 .epoch = 1000};
experiment e5 = {.alpha = 3825,
                 .firing = 639,
                 .lr_p_inp = 12,
                 .lr_p_rec = 11,
                 .lr_p_out = 13,
                 .epoch = 100};
experiment e6 = {.alpha = 3943,
                 .firing = 253,
                 .lr_p_inp = 13,
                 .lr_p_rec = 10,
                 .lr_p_out = 13,
                 .epoch = 1000};
experiment e8 = {.alpha = 3845,
                 .firing = 366,
                 .lr_p_inp = 13,
                 .lr_p_rec = 9,
                 .lr_p_out = 14,
                 .epoch = 500};
experiment e4 = {.alpha = 3825,
                 .firing = 641,
                 .lr_p_inp = 8,
                 .lr_p_rec = 8,
                 .lr_p_out = 12,
                 .epoch = 100};

#pragma GCC diagnostic pop
#define CHECK_CYNQ_ERROR(err) \
  if (err.code != 0) std::cerr << "[CYNQ ERR]: " << err.msg << std::endl;
#define DELAYED_EXECUTION(err, delay) \
  usleep(delay);                      \
  if (err.code != 0) std::cerr << "[CYNQ ERR]: " << err.msg << std::endl;

#define SLEEP(delay) usleep(delay);

static cynq::Status __attribute__((optimize("O0")))
spi_write_word(std::shared_ptr<cynq::IAccelerator> spi, uint64_t addr,
               uint32_t val) {
  cynq::Status err;

  err = spi->Write(addr, &val);
  SLEEP(1);
  CHECK_CYNQ_ERROR(err);
  return err;
}
static double __attribute__((optimize("O0"))) get_timestamp() {
  double time = std::chrono::duration_cast<std::chrono::duration<double>>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  return time;
}

static uint32_t __attribute__((optimize("O0")))
spi_read_word(std::shared_ptr<cynq::IAccelerator> spi, uint64_t addr,
              bool = false) {
  uint32_t val = 0;
  SLEEP(1);
  CHECK_CYNQ_ERROR(spi->Read(addr, &val));
  return val;
}

static void __attribute__((optimize("O0")))
config_spi(std::shared_ptr<cynq::IAccelerator> spi) {
  std::cout << "Configure device" << std::endl;
  uint32_t ControlReg = 0;

  // Reset the SPI device
  spi_write_word(spi, XSP_SRR_OFFSET, XSP_SRR_RESET_MASK);
  // Enable the transmit empty interrupt, which we use to determine progress on
  // the transmission.
  spi_write_word(spi, XSP_IIER_OFFSET,
                 XSP_SR_TX_EMPTY_MASK);  // XSP_SR_TX_EMPTY_MASK
  // Disable the global IPIF interrupt
  spi_write_word(spi, XSP_DGIER_OFFSET, 0);
  // Deselect the slave on the SPI bus
  spi_write_word(spi, XSP_SSR_OFFSET, SLAVE_NO_SELECTION);
  // Disable the transmitter, enable Manual Slave Select Assertion, put SPI
  // controller into master mode, and enable it
  ControlReg = spi_read_word(spi, XSP_CR_OFFSET);
  ControlReg = ControlReg | XSP_CR_MASTER_MODE_MASK | XSP_CR_MANUAL_SS_MASK |
               XSP_CR_ENABLE_MASK | XSP_CR_TXFIFO_RESET_MASK |
               XSP_CR_RXFIFO_RESET_MASK;
  spi_write_word(spi, XSP_CR_OFFSET, ControlReg);
  ControlReg = spi_read_word(spi, XSP_CR_OFFSET);
  ControlReg = ControlReg & ~(XSP_CR_CLK_PHASE_MASK | XSP_CR_CLK_POLARITY_MASK);
  spi_write_word(spi, XSP_CR_OFFSET, ControlReg);
}

constexpr uint GPIOS_IN = 0;
constexpr uint GPIOS_OUT = 1;

enum class GpioTransaction {
  TIME_TICK_high,
  TIME_TICK_low,
  INFER_ACC_high,
  INFER_ACC_low,
  TARGET_VALID_high,
  TARGET_VALID_low,
  SAMPLE_high,
  SAMPLE_low,
  Select_AERIN_pin,
  Select_AERIN_gpio,
  TEST_high,
  TEST_low,
  AERINTAREN_high,
  AERINTAREN_low,
  ACKOUT_high,
  ACKOUT_low,
  AERIN_data,
  RST_high,
  RST_low,
  SPIRDY_read,
  TIMINGERROR_read,
  AERout_read,
  REQOUT_read,
  OK_read,
  AEROUTbus_read,
};

static uint32_t gpio_transaction(std::shared_ptr<cynq::AXIGPIO> gpio,
                                 GpioTransaction trans, uint32_t data = 0) {
  SLEEP(1);
  uint32_t val = 0;
  switch (trans) {
    case GpioTransaction::TIME_TICK_high: {
      val = 0x1;
      gpio->Write(GPIOS_OUT, 0, 1, val);
    } break;
    case GpioTransaction::TIME_TICK_low: {
      val = 0x0;
      gpio->Write(GPIOS_OUT, 0, 1, val);
    } break;
    case GpioTransaction::INFER_ACC_high: {
      val = 0x1;
      gpio->Write(GPIOS_OUT, 1, 2, val);
    } break;
    case GpioTransaction::INFER_ACC_low: {
      val = 0x0;
      gpio->Write(GPIOS_OUT, 1, 2, val);
    } break;
    case GpioTransaction::TARGET_VALID_high: {
      val = 0x1;
      gpio->Write(GPIOS_OUT, 2, 3, val);
    } break;
    case GpioTransaction::TARGET_VALID_low: {
      val = 0x0;
      gpio->Write(GPIOS_OUT, 2, 3, val);
    } break;
    case GpioTransaction::SAMPLE_high: {
      val = 0x1;
      gpio->Write(GPIOS_OUT, 3, 4, val);
    } break;
    case GpioTransaction::SAMPLE_low: {
      val = 0x0;
      gpio->Write(GPIOS_OUT, 3, 4, val);
    } break;
    case GpioTransaction::Select_AERIN_pin: {
      val = 0x1;
      gpio->Write(GPIOS_OUT, 5, 6, val);
    } break;
    case GpioTransaction::Select_AERIN_gpio: {
      val = 0x0;
      gpio->Write(GPIOS_OUT, 5, 6, val);
    } break;
    case GpioTransaction::TEST_high: {
      val = 0x1;
      gpio->Write(GPIOS_OUT, 6, 7, val);
    } break;
    case GpioTransaction::TEST_low: {
      val = 0x0;
      gpio->Write(GPIOS_OUT, 6, 7, val);
    } break;
    case GpioTransaction::AERINTAREN_high: {
      val = 0x1;
      gpio->Write(GPIOS_OUT, 7, 8, val);
    } break;
    case GpioTransaction::AERINTAREN_low: {
      val = 0x0;
      gpio->Write(GPIOS_OUT, 7, 8, val);
    } break;
    case GpioTransaction::ACKOUT_high: {
      val = 0x1;
      gpio->Write(GPIOS_OUT, 8, 9, val);
    } break;
    case GpioTransaction::ACKOUT_low: {
      val = 0x0;
      gpio->Write(GPIOS_OUT, 8, 9, val);
    } break;
    case GpioTransaction::AERIN_data: {
      val = data;
      gpio->Write(GPIOS_OUT, 9, 15, val);
    } break;
    case GpioTransaction::RST_high: {
      val = 0x1;
      gpio->Write(GPIOS_OUT, 15, 16, val);
    } break;
    case GpioTransaction::RST_low: {
      val = 0x0;
      gpio->Write(GPIOS_OUT, 15, 16, val);
    } break;
    case GpioTransaction::SPIRDY_read:
      gpio->Read(GPIOS_IN, 1, 2, val);
      break;
    case GpioTransaction::TIMINGERROR_read:
      gpio->Read(GPIOS_IN, 2, 3, val);
      break;
    case GpioTransaction::AERout_read:
      gpio->Read(GPIOS_IN, 4, 5, val);
      break;
    case GpioTransaction::REQOUT_read:
      gpio->Read(GPIOS_IN, 3, 4, val);
      break;
    case GpioTransaction::OK_read:
      gpio->Read(GPIOS_IN, 0, 1, val);
      break;
    case GpioTransaction::AEROUTbus_read:
      gpio->Read(GPIOS_IN, 5, 9, val);
      break;
    default:
      break;
  }

  return val;
}

std::vector<uint32_t> __attribute__((optimize("O0")))
xfer(std::shared_ptr<cynq::IAccelerator> spi, uint32_t *packet, uint32_t len) {
  uint32_t ControlReg = 0, StatusReg = 0, RxFifoStatus = 0, temp = 0;

  for (uint i = 0; i < len; i++) {
    spi_write_word(spi, XSP_DTR_OFFSET, packet[i]);
  }

  spi_write_word(spi, XSP_SSR_OFFSET, 0xFFFFFFFE);
  ControlReg = spi_read_word(spi, XSP_CR_OFFSET);
  ControlReg = ControlReg & ~XSP_CR_TRANS_INHIBIT_MASK;
  spi_write_word(spi, XSP_CR_OFFSET, ControlReg);

  StatusReg = spi_read_word(spi, XSP_SR_OFFSET);

  while ((StatusReg & XSP_SR_TX_EMPTY_MASK) == 0) {
    StatusReg = spi_read_word(spi, XSP_SR_OFFSET);
  }

  ControlReg = spi_read_word(spi, XSP_CR_OFFSET);
  ControlReg = ControlReg | XSP_CR_TRANS_INHIBIT_MASK;
  spi_write_word(spi, XSP_CR_OFFSET, ControlReg);

  spi_write_word(spi, XSP_SSR_OFFSET, SLAVE_NO_SELECTION);

  std::vector<uint32_t> resp;
  RxFifoStatus = spi_read_word(spi, XSP_SR_OFFSET) & 0x01;
  while (RxFifoStatus == 0) {
    temp = spi_read_word(spi, XSP_RFO_OFFSET);
    temp = spi_read_word(spi, XSP_DRR_OFFSET);
    resp.push_back(temp);
    RxFifoStatus = spi_read_word(spi, XSP_SR_OFFSET) & 0x01;
  }

  return resp;
}

static std::vector<uint32_t> spi_send(std::shared_ptr<cynq::IAccelerator> spi,
                                      uint32_t addr, uint32_t data) {
  uint32_t packet[2] = {addr, data};
  return xfer(spi, packet, 2);
}

[[maybe_unused]] static std::vector<uint32_t> spi_read(
    std::shared_ptr<cynq::IAccelerator> spi, uint32_t addr, uint32_t /*data*/) {
  uint32_t packet[2] = {addr};
  return xfer(spi, packet, 1);
}

static std::vector<uint32_t> spi_half_send(
    std::shared_ptr<cynq::IAccelerator> spi, uint32_t data) {
  return xfer(spi, &data, 1);
}

static void __attribute__((optimize("O0")))
send_snn_parameters(std::shared_ptr<cynq::IAccelerator> spi) {
  spi_send(spi, 0x00010000, SPI_EN_CONF);
  spi_send(spi, 0x00010001, SPI_RO_STAGE_SEL);
  spi_send(spi, 0x00010002, SPI_GET_CLKINT_OUT);
  spi_send(spi, 0x00010008, SPI_RST_MODE);
  spi_send(spi, 0x00010009, SPI_DO_EPROP);
  spi_send(spi, 0x0001000A, SPI_LOCAL_TICK);
  spi_send(spi, 0x0001000B, SPI_ERROR_HALT);
  spi_send(spi, 0x0001000C, SPI_FP_LOC_WINP);
  spi_send(spi, 0x0001000D, SPI_FP_LOC_WREC);
  spi_send(spi, 0x0001000E, SPI_FP_LOC_WOUT);
  spi_send(spi, 0x0001000F, SPI_FP_LOC_TINP);
  spi_send(spi, 0x00010010, SPI_FP_LOC_TREC);
  spi_send(spi, 0x00010011, SPI_FP_LOC_TOUT);
  spi_send(spi, 0x00010012, SPI_LEARN_SIG_SCALE);
  spi_send(spi, 0x00010013, SPI_REGUL_MODE);
  spi_send(spi, 0x00010014, SPI_REGUL_W);
  spi_send(spi, 0x00010015, SPI_EN_STOCH_ROUND);
  spi_send(spi, 0x00010016, SPI_SRAM_SPEEDMODE);
  spi_send(spi, 0x00010017, SPI_TIMING_MODE);
  spi_send(spi, 0x00010019, SPI_REGRESSION);
  spi_send(spi, 0x0001001A, SPI_SINGLE_LABEL);
  spi_send(spi, 0x0001001B, SPI_NO_OUT_ACT);
  spi_send(spi, 0x0001001E, SPI_SEND_PER_TIMESTEP);
  spi_send(spi, 0x0001001F, SPI_SEND_LABEL_ONLY);
  spi_send(spi, 0x00010020, SPI_NOISE_EN);
  spi_send(spi, 0x00010021, SPI_FORCE_TRACES);
  spi_send(spi, 0x00010040, SPI_CYCLES_PER_TICK);
  spi_send(spi, 0x00010041, SPI_ALPHA_CONF0);
  spi_send(spi, 0x00010042, SPI_ALPHA_CONF1);
  spi_send(spi, 0x00010043, SPI_ALPHA_CONF2);
  spi_send(spi, 0x00010044, SPI_ALPHA_CONF3);
  spi_send(spi, 0x00010045, SPI_KAPPA);
  spi_send(spi, 0x00010046, SPI_THR_H_0);
  spi_send(spi, 0x00010047, SPI_THR_H_1);
  spi_send(spi, 0x00010048, SPI_THR_H_2);
  spi_send(spi, 0x00010049, SPI_THR_H_3);
  spi_send(spi, 0x0001004A, SPI_H_0);
  spi_send(spi, 0x0001004B, SPI_H_1);
  spi_send(spi, 0x0001004C, SPI_H_2);
  spi_send(spi, 0x0001004D, SPI_H_3);
  spi_send(spi, 0x0001004E, SPI_H_4);
  spi_send(spi, 0x0001004F, SPI_LR_R_WINP);
  spi_send(spi, 0x00010050, SPI_LR_P_WINP);
  spi_send(spi, 0x00010051, SPI_LR_R_WREC);
  spi_send(spi, 0x00010052, SPI_LR_P_WREC);
  spi_send(spi, 0x00010053, SPI_LR_R_WOUT);
  spi_send(spi, 0x00010054, SPI_LR_P_WOUT);
  spi_send(spi, 0x00010055, SPI_SEED_INP);
  spi_send(spi, 0x00010056, SPI_SEED_REC);
  spi_send(spi, 0x00010057, SPI_SEED_OUT);
  spi_send(spi, 0x00010058, SPI_SEED_STRND_NEUR);
  spi_send(spi, 0x00010059, SPI_SEED_STRND_ONEUR);
  spi_send(spi, 0x0001005A, SPI_SEED_STRND_TINP);
  spi_send(spi, 0x0001005B, SPI_SEED_STRND_TREC);
  spi_send(spi, 0x0001005C, SPI_SEED_STRND_TOUT);
  spi_send(spi, 0x0001005D, SPI_SEED_NOISE_NEUR);
  spi_send(spi, 0x0001005E, SPI_NUM_INP_NEUR);
  spi_send(spi, 0x0001005F, SPI_NUM_REC_NEUR);
  spi_send(spi, 0x00010060, SPI_NUM_OUT_NEUR);
  spi_send(spi, 0x00010062, SPI_REGUL_F0);
  spi_send(spi, 0x00010063, SPI_REGUL_K_INP_R);
  spi_send(spi, 0x00010064, SPI_REGUL_K_INP_P);
  spi_send(spi, 0x00010065, SPI_REGUL_K_REC_R);
  spi_send(spi, 0x00010066, SPI_REGUL_K_REC_P);
  spi_send(spi, 0x00010067, SPI_REGUL_K_MUL);
  spi_send(spi, 0x00010068, SPI_NOISE_STR);
  spi_send(spi, 0x00010000, SPI_DIS_CONF);
}

[[maybe_unused]] static void check_status(std::shared_ptr<cynq::AXIGPIO> gpio) {
  std::cout << "OK= " << gpio_transaction(gpio, GpioTransaction::OK_read)
            << std::endl;
  std::cout << "REQOUT= "
            << gpio_transaction(gpio, GpioTransaction::REQOUT_read)
            << std::endl;
  std::cout << "TIMINGERROR= "
            << gpio_transaction(gpio, GpioTransaction::TIMINGERROR_read)
            << std::endl;
  std::cout << "SPIRDY= "
            << gpio_transaction(gpio, GpioTransaction::SPIRDY_read)
            << std::endl;
  std::cout << "AEROUTbus= "
            << gpio_transaction(gpio, GpioTransaction::AEROUTbus_read)
            << std::endl;
  std::cout << "REQOUT= "
            << gpio_transaction(gpio, GpioTransaction::REQOUT_read)
            << std::endl;
  std::cout << "SPIRDY= "
            << gpio_transaction(gpio, GpioTransaction::SPIRDY_read)
            << std::endl;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
static void transmit_input(std::shared_ptr<cynq::IAccelerator> spi, uint Ninp,
                           uint Nrec, int32_t *buffer,
                           uint32_t /*buffer_size*/) {  // NOLINT
  uint32_t counter = 0;
  for (int32_t i = 0; i < (int32_t)Ninp; i++) {                      // NOLINT
    spi_half_send(spi, (0x30 << 24 | ((int32_t)(Nrec / 4) << 16)) |  // NOLINT
                           (0x3fc0 & (i << 6)));                     // NOLINT
    for (uint32_t j = 0; j < (uint32_t)(Nrec / 4); j++) {            // NOLINT
      uint32_t prog_val32 = 0;
      for (uint k = 0; k < 4; k++) {
        int32_t weight = buffer[counter];
        counter++;
        prog_val32 |= (weight & 0x000000FF) << (8 * k);  // NOLINT
      }
      spi_half_send(spi, prog_val32);
    }
    if (counter >= 4000) {
      counter = 0;
      continue;
    }
  }
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
static void transmit_rec(std::shared_ptr<cynq::IAccelerator> spi, uint Nrec,
                         int32_t *buffer, uint32_t /*buffer_size*/) {  // NOLINT
  SLEEP(10);
  uint32_t counter = 0;
  for (int32_t i = 0; i < (int32_t)Nrec; i++) {                    // NOLINT
    spi_half_send(spi, (0x40 << 24 | (int32_t)(Nrec / 4) << 16) |  // NOLINT
                           (0x3fc0 & ((uint32_t)i << 6)));         // NOLINT
    for (int32_t j = 0; j < (int32_t)(Nrec / 4); j++) {            // NOLINT
      uint32_t prog_val32 = 0;
      for (uint32_t k = 0; k < 4; k++) {
        int32_t weight = buffer[counter];  // NOLINT
        counter++;
        prog_val32 |= (weight & 0x000000FF) << (8 * k);  // NOLINT
      }
      spi_half_send(spi, prog_val32);
    }
    if (counter >= 10000) {
      counter = 0;
      continue;
    }
  }
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
static void transmit_out(std::shared_ptr<cynq::IAccelerator> spi,
                         uint32_t Nrec,  // NOLINT
                         uint Nout, int32_t *buffer,
                         uint32_t /*buffer_size*/) {  // NOLINT
  int counter = 0;
  for (uint32_t i = 0; i < Nrec; i++) {
    uint32_t prog_val32 = 0;
    for (uint32_t k = 0; k < Nout; k++) {
      int32_t weight = buffer[counter];  // NOLINT
      counter++;
      prog_val32 |= (weight & 0x000000FF) << (8 * k);  // NOLINT
    }
    uint32_t kk = 0x50010000 | ((0xFF & i) << 2);  // NOLINT
    spi_send(spi, kk, prog_val32);
    if (counter >= 200) {
      counter = 0;
      continue;
    }
  }
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
static void initialise_neurons(std::shared_ptr<cynq::IAccelerator> spi,
                               uint32_t Nrec, uint32_t Alpha_12lsb,
                               uint32_t Firing_TH) {  // NOLINT
  uint32_t prog_val128[4] = {
      ((Alpha_12lsb << 20) | (Firing_TH << 4)),  // NOLINT
      0x0, 0x0, 0x0};
  spi_half_send(spi, (0x1 << 28 | ((Nrec * 4) << 16)));  // NOLINT
  for (uint32_t i = 0; i < Nrec; i++) {
    for (uint32_t j = 0; j < 4; j++) {
      uint32_t prog_val32 = prog_val128[3 - j];  // NOLINT
      spi_half_send(spi, prog_val32);
    }
  }
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
[[maybe_unused]] static std::vector<uint32_t> read_neurons(
    std::shared_ptr<cynq::IAccelerator> spi,
    uint32_t Nrec) {  // NOLINT
  std::vector<uint32_t> output;
  uint32_t address =
      ((uint32_t)(0x9) << (uint32_t)(28) | ((Nrec * 4) << 16));  // NOLINT
  for (uint32_t i = 0; i < Nrec; i++) {
    for (uint32_t j = 0; j < 4; j++) {
      uint32_t address_r = address | (i * 4 + j);  // NOLINT
      output.push_back(spi_read_word(spi, address_r));
    }
  }
  return output;
}
#pragma GCC diagnostic pop

std::vector<std::vector<double>> __attribute__((optimize("O0"))) singleRun() {
  // NOTE: This is a basic example. Error checking has been removed to keep
  // simplicity but it is always recommended
  using namespace cynq;  // NOLINT
  // py::scoped_interpreter guard{};
  // Create the platform
  std::shared_ptr<IHardware> platform =
      IHardware::Create(HardwareArchitecture::UltraScale);
  // Get IP cores involved
  std::vector<double> trainData;
  std::vector<double> testData;
  std::vector<double> afterTrainData;

  auto spi = platform->GetAccelerator(0x80000000);
  auto gpio = std::make_shared<AXIGPIO>(0x80010000);
  auto dma = platform->GetDataMover(0x80020000);

  std::vector<char> train_file_buffer;
  std::vector<char> test_file_buffer;
  std::vector<int32_t> train_data;
  std::vector<int32_t> test_data;

  std::ifstream train_file(
      "/home/ubuntu/cynq/examples/zynq-mpsoc/reckon/train_wb.bin",
      std::ios::binary);
  train_file_buffer =
      std::vector<char>(std::istreambuf_iterator<char>(train_file), {});
  size_t train_size = train_file_buffer.size() / sizeof(int32_t);
  int32_t *train_ptr_temp =
      reinterpret_cast<int32_t *>(train_file_buffer.data());
  train_data.assign(train_ptr_temp, train_ptr_temp + train_size);

  std::ifstream test_file(
      "/home/ubuntu/cynq/examples/zynq-mpsoc/reckon/test_wb.bin",
      std::ios::binary);
  test_file_buffer =
      std::vector<char>(std::istreambuf_iterator<char>(test_file), {});
  size_t test_size = test_file_buffer.size() / sizeof(int32_t);
  int32_t *test_ptr_temp = reinterpret_cast<int32_t *>(test_file_buffer.data());
  test_data.assign(test_ptr_temp, test_ptr_temp + test_size);

  std::ifstream inp_file(
      "/home/ubuntu/cynq/examples/zynq-mpsoc/reckon/"
      "init_rand_weights_inp_cueAcc.bin",
      std::ios::binary);
  std::ifstream rec_file(
      "/home/ubuntu/cynq/examples/zynq-mpsoc/reckon/"
      "init_rand_weights_rec_cueAcc.bin",
      std::ios::binary);
  std::ifstream out_file(
      "/home/ubuntu/cynq/examples/zynq-mpsoc/reckon/"
      "init_rand_weights_out_cueAcc.bin",
      std::ios::binary);

  // Copy the data
  std::vector<char> inp_buffer(std::istreambuf_iterator<char>(inp_file), {});
  std::vector<char> rec_buffer(std::istreambuf_iterator<char>(rec_file), {});
  std::vector<char> out_buffer(std::istreambuf_iterator<char>(out_file), {});

  int32_t *test_ptr = test_data.data();
  int32_t *train_ptr = train_data.data();
  int32_t *inp_ptr = reinterpret_cast<int32_t *>(inp_buffer.data());
  int32_t *rec_ptr = reinterpret_cast<int32_t *>(rec_buffer.data());
  int32_t *out_ptr = reinterpret_cast<int32_t *>(out_buffer.data());

  experiment ex = e4;
  SPI_LR_P_WINP = ex.lr_p_inp;
  SPI_LR_P_WREC = ex.lr_p_rec;
  SPI_LR_P_WOUT = ex.lr_p_out;
  uint32_t inpNeur = SPI_NUM_INP_NEUR + 1;  // NOLINT
  uint32_t recNeur = SPI_NUM_REC_NEUR + 1;  // NOLINT
  uint32_t outNeur = SPI_NUM_OUT_NEUR + 1;  // NOLINT
  // experiment parameters

  uint32_t Alpha_12lsb = ex.alpha;
  uint32_t Firing_TH = ex.firing;
  uint32_t epochs = ex.epoch;

  // Configure
  auto configStart = get_timestamp();
  config_spi(spi);
  auto configEnd = get_timestamp();
  std::cout << "SPI Configuration time: " << (configEnd - configStart)
            << " seconds" << std::endl;
  // Check configuration
  send_snn_parameters(spi);
  spi_send(spi, 0x00010000, SPI_EN_CONF);
  transmit_input(spi, inpNeur, recNeur, inp_ptr, inp_buffer.size());
  SLEEP(1);
  transmit_rec(spi, recNeur, rec_ptr, rec_buffer.size());
  SLEEP(1);
  transmit_out(spi, recNeur, outNeur, out_ptr, out_buffer.size());
  SLEEP(1);
  initialise_neurons(spi, recNeur, Alpha_12lsb, Firing_TH);
  spi_send(spi, 0x00010000, 0x00000000);

  std::cout << "Configuration time: " << (configEnd - configStart) << " seconds"
            << std::endl;
  // Create buffer

  std::cout << "Training-Test Loop" << std::endl;
  auto input_buffer = dma->GetBuffer(1000 * sizeof(uint32_t));
  uint32_t *input_buffer_hptr = input_buffer->HostAddress<uint32_t>().get();
  auto send_buffer = dma->GetBuffer(1000 * sizeof(uint32_t));
  uint32_t *send_buffer_hptr = send_buffer->HostAddress<uint32_t>().get();

  double beggining = get_timestamp();
  for (uint epoch = 0; epoch <= epochs; epoch++) {
    EpochData trainEpoch;
    EpochData testEpoch;
    EpochData afterTrainEpoch;
    trainEpoch.number = epoch;
    testEpoch.number = epoch;
    afterTrainEpoch.number = epoch;
    for (uint test = 0; test <= 2; test++) {
      int32_t *ptr = nullptr;
      uint32_t correct = 0;
      uint32_t sample = 0;
      std::string trainingPhase;
      spi_send(spi, 0x00010000, 0x00000001);
      if (test == 0) {
        trainingPhase = "training";
        spi_send(spi, 0x00010009, 0x00000007);
        ptr = train_ptr;
        gpio_transaction(gpio, GpioTransaction::TEST_low);
      } else if (test == 1) {
        trainingPhase = "test";
        spi_send(spi, 0x00010009, 0x00000000);
        ptr = train_ptr;
        gpio_transaction(gpio, GpioTransaction::TEST_high);
      } else {
        trainingPhase = "testAfterTraining";
        spi_send(spi, 0x00010009, 0x00000000);
        ptr = test_ptr;
        gpio_transaction(gpio, GpioTransaction::TEST_high);
      }

      spi_send(spi, 0x00010000, 0x00000000);
      uint32_t len = (uint32_t)ptr[sample];  // NOLINT
      sample++;
      int32_t evt_target = 0;
      double global_sample_time = 0.0;
      double global_dma_time = 0.0;

      for (uint curr_sample = 0; curr_sample < len; curr_sample++) {
        double start = get_timestamp();
        for (uint32_t k = 0; k < 1000; k++) {
          input_buffer_hptr[k] = 0x0u;
          send_buffer_hptr[k] = 0x0u;
        }

        dma->Upload(send_buffer, send_buffer->Size(), 0, ExecutionType::Sync);

        int32_t evt_neur = ptr[sample++];
        int32_t evt_time = ptr[sample++];

        uint32_t i = 0;

        while (true) {
          uint32_t res = ((uint32_t)(evt_neur) << 16) |  // NOLINT
                         ((uint32_t)(evt_time)&0xFFFF);  // NOLINT
          input_buffer_hptr[i] = res;

          i++;

          if (evt_neur == -1) break;
          if (evt_neur == -2) evt_target = evt_time;

          evt_neur = ptr[sample++];
          evt_time = ptr[sample++];
        }

        for (uint32_t j = 0; j < i; j++) {
          send_buffer_hptr[j] = input_buffer_hptr[j];
        }

        dma->Upload(input_buffer, input_buffer->Size(), 0, ExecutionType::Sync);
        double endDma = get_timestamp();
        uint32_t kk = gpio_transaction(gpio, GpioTransaction::OK_read);

        while (!kk) {
          kk = gpio_transaction(gpio, GpioTransaction::OK_read);
        }

        double inference_time = get_timestamp();
        int32_t inference =
            gpio_transaction(gpio, GpioTransaction::AEROUTbus_read);  // NOLINT

        correct += (uint32_t)(inference == evt_target);  // NOLINT
        global_sample_time =
            global_sample_time + (inference_time - start);     // NOLINT
        global_dma_time = global_dma_time + (endDma - start);  // NOLINT
      }

      double precision = correct / (double)len;  // NOLINT
      double dmaTimes = global_dma_time;         // NOLINT
      double sampleTimes = global_sample_time;   // NOLINT

      if (test == 2) {
        testEpoch.precision = precision;      //	 NOLINT
        testEpoch.dmaTimes = dmaTimes;        // NOLINT
        testEpoch.sampleTimes = sampleTimes;  // NOLINT
      } else if (test == 0) {
        trainEpoch.precision = precision;      // NOLINT
        trainEpoch.dmaTimes = dmaTimes;        // NOLINT
        trainEpoch.sampleTimes = sampleTimes;  // NOLINT
      } else {
        afterTrainEpoch.precision = precision;      // NOLINT
        afterTrainEpoch.dmaTimes = dmaTimes;        // NOLINT
        afterTrainEpoch.sampleTimes = sampleTimes;  // NOLINT
      }
    }

    trainData.push_back(trainEpoch.precision);            // NOLINT
    testData.push_back(testEpoch.precision);              // NOLINT
    afterTrainData.push_back(afterTrainEpoch.precision);  // NOLINT
  }
  double ending = get_timestamp();
  double total_time = ending - beggining;
  std::cout << "Finished experiment" << std::endl;
  std::cout << "Total time: " << total_time << std::endl;
  std::cout << "\n";
  std::vector<std::vector<double>> data;
  data.push_back(testData);
  data.push_back(trainData);
  data.push_back(afterTrainData);
  return data;
}

void __attribute__((optimize("O0")))
trainReckon(int first, py::dict &input, py::function &callback) {
  // NOTE: This is a basic example. Error checking has been removed to keep
  // simplicity but it is always recommended
  using namespace cynq;  // NOLINT
  // Create the platform
  std::shared_ptr<IHardware> platform =
      IHardware::Create(HardwareArchitecture::UltraScale);

  // Get IP cores involved

  auto spi = platform->GetAccelerator(0x80000000);
  auto gpio = std::make_shared<AXIGPIO>(0x80010000);
  auto dma = platform->GetDataMover(0x80020000);

  std::vector<char> train_file_buffer;
  std::vector<char> test_file_buffer;
  std::vector<int32_t> train_data;
  std::vector<int32_t> test_data;

  std::ifstream train_file(
      "/home/ubuntu/cynq/examples/zynq-mpsoc/reckon/train_wb.bin",
      std::ios::binary);
  train_file_buffer =
      std::vector<char>(std::istreambuf_iterator<char>(train_file), {});
  size_t train_size = train_file_buffer.size() / sizeof(int32_t);
  int32_t *train_ptr_temp =
      reinterpret_cast<int32_t *>(train_file_buffer.data());
  train_data.assign(train_ptr_temp, train_ptr_temp + train_size);

  std::ifstream test_file(
      "/home/ubuntu/cynq/examples/zynq-mpsoc/reckon/test_wb.bin",
      std::ios::binary);
  test_file_buffer =
      std::vector<char>(std::istreambuf_iterator<char>(test_file), {});
  size_t test_size = test_file_buffer.size() / sizeof(int32_t);
  int32_t *test_ptr_temp = reinterpret_cast<int32_t *>(test_file_buffer.data());
  test_data.assign(test_ptr_temp, test_ptr_temp + test_size);

  std::ifstream inp_file(
      "/home/ubuntu/cynq/examples/zynq-mpsoc/reckon/"
      "init_rand_weights_inp_cueAcc.bin",
      std::ios::binary);
  std::ifstream rec_file(
      "/home/ubuntu/cynq/examples/zynq-mpsoc/reckon/"
      "init_rand_weights_rec_cueAcc.bin",
      std::ios::binary);
  std::ifstream out_file(
      "/home/ubuntu/cynq/examples/zynq-mpsoc/reckon/"
      "init_rand_weights_out_cueAcc.bin",
      std::ios::binary);

  // Copy the data
  std::vector<char> inp_buffer(std::istreambuf_iterator<char>(inp_file), {});
  std::vector<char> rec_buffer(std::istreambuf_iterator<char>(rec_file), {});
  std::vector<char> out_buffer(std::istreambuf_iterator<char>(out_file), {});

  int32_t *test_ptr = test_data.data();
  int32_t *train_ptr = train_data.data();
  int32_t *inp_ptr = reinterpret_cast<int32_t *>(inp_buffer.data());
  int32_t *rec_ptr = reinterpret_cast<int32_t *>(rec_buffer.data());
  int32_t *out_ptr = reinterpret_cast<int32_t *>(out_buffer.data());

  uint32_t alpha = input["alpha"].cast<uint32_t>();        // NOLINT
  uint32_t firing = input["firing"].cast<uint32_t>();      // NOLINT
  uint32_t lr_p_inp = input["lr_p_inp"].cast<uint32_t>();  // NOLINT
  uint32_t lr_p_rec = input["lr_p_rec"].cast<uint32_t>();  // NOLINT
  uint32_t lr_p_out = input["lr_p_out"].cast<uint32_t>();  // NOLINT
  uint32_t epoch = input["epochs"].cast<uint32_t>();       // NOLINT
  experiment experimentInput = {.alpha = alpha,
                                .firing = firing,
                                .lr_p_inp = lr_p_inp,
                                .lr_p_rec = lr_p_rec,
                                .lr_p_out = lr_p_out,
                                .epoch = epoch};
  experiment ex;
  if (first == 0) {
    ex = e4;
  } else {
    ex = experimentInput;
  }
  uint32_t Alpha_12lsb = ex.alpha;
  uint32_t Firing_TH = ex.firing;
  uint32_t epochs = ex.epoch;

  SPI_LR_P_WINP = ex.lr_p_inp;
  SPI_LR_P_WREC = ex.lr_p_rec;
  SPI_LR_P_WOUT = ex.lr_p_out;
  uint32_t inpNeur = SPI_NUM_INP_NEUR + 1;  // NOLINT
  uint32_t recNeur = SPI_NUM_REC_NEUR + 1;  // NOLINT
  uint32_t outNeur = SPI_NUM_OUT_NEUR + 1;  // NOLINT
  // Configure

  config_spi(spi);

  // Check configuration
  send_snn_parameters(spi);
  spi_send(spi, 0x00010000, SPI_EN_CONF);
  transmit_input(spi, inpNeur, recNeur, inp_ptr, inp_buffer.size());
  SLEEP(1);
  transmit_rec(spi, recNeur, rec_ptr, rec_buffer.size());
  SLEEP(1);
  transmit_out(spi, recNeur, outNeur, out_ptr, out_buffer.size());
  SLEEP(1);
  initialise_neurons(spi, recNeur, Alpha_12lsb, Firing_TH);
  spi_send(spi, 0x00010000, 0x00000000);

  // Create buffer

  auto input_buffer = dma->GetBuffer(1000 * sizeof(uint32_t));
  uint32_t *input_buffer_hptr = input_buffer->HostAddress<uint32_t>().get();
  auto send_buffer = dma->GetBuffer(1000 * sizeof(uint32_t));
  uint32_t *send_buffer_hptr = send_buffer->HostAddress<uint32_t>().get();

  double beggining = get_timestamp();
  for (uint epoch = 0; epoch <= epochs; epoch++) {
    EpochData trainEpoch;
    EpochData testEpoch;
    EpochData afterTrainEpoch;
    trainEpoch.number = epoch;
    testEpoch.number = epoch;
    afterTrainEpoch.number = epoch;
    for (uint test = 0; test <= 2; test++) {
      int32_t *ptr = nullptr;
      uint32_t correct = 0;
      uint32_t sample = 0;
      std::string trainingPhase;
      spi_send(spi, 0x00010000, 0x00000001);
      if (test == 0) {
        trainingPhase = "training";
        spi_send(spi, 0x00010009, 0x00000007);
        ptr = train_ptr;
        gpio_transaction(gpio, GpioTransaction::TEST_low);
      } else if (test == 2) {
        trainingPhase = "test";
        spi_send(spi, 0x00010009, 0x00000000);
        ptr = train_ptr;
        gpio_transaction(gpio, GpioTransaction::TEST_high);
      } else {
        trainingPhase = "testAfterTraining";
        spi_send(spi, 0x00010009, 0x00000000);
        ptr = test_ptr;
        gpio_transaction(gpio, GpioTransaction::TEST_high);
      }

      spi_send(spi, 0x00010000, 0x00000000);
      uint32_t len = (uint32_t)ptr[sample];  // NOLINT

      sample++;
      int32_t evt_target = 0;
      double global_sample_time = 0.0;
      double global_dma_time = 0.0;

      for (uint curr_sample = 0; curr_sample < len; curr_sample++) {
        double start = get_timestamp();
        for (uint32_t k = 0; k < 1000; k++) {
          send_buffer_hptr[k] = 0;
          input_buffer_hptr[k] = 0;
        }
        dma->Upload(send_buffer, send_buffer->Size(), 0, ExecutionType::Sync);
        int32_t evt_neur = ptr[sample++];
        int32_t evt_time = ptr[sample++];

        uint32_t i = 0;

        while (true) {
          input_buffer_hptr[i] =
              (uint32_t)((uint32_t)(evt_neur * 65536) +  // NOLINT
                         (uint32_t)evt_time);            // NOLINT
          i++;

          if (evt_neur == -1) break;
          if (evt_neur == -2) evt_target = evt_time;

          evt_neur = ptr[sample++];
          evt_time = ptr[sample++];
        }

        for (uint32_t j = 0; j < i; j++) {
          send_buffer_hptr[j] = input_buffer_hptr[j];
        }

        dma->Upload(input_buffer, input_buffer->Size(), 0, ExecutionType::Sync);
        double endDma = get_timestamp();
        uint32_t kk = gpio_transaction(gpio, GpioTransaction::OK_read);

        while (!kk) {
          kk = gpio_transaction(gpio, GpioTransaction::OK_read);
        }

        double inference_time = get_timestamp();
        int32_t inference =
            gpio_transaction(gpio, GpioTransaction::AEROUTbus_read);  // NOLINT

        correct += (uint32_t)(inference == evt_target);  // NOLINT
        global_sample_time =
            global_sample_time + (inference_time - start);     // NOLINT
        global_dma_time = global_dma_time + (endDma - start);  // NOLINT
      }

      double precision = correct / (double)len;  // NOLINT
      double dmaTimes = global_dma_time;         // NOLINT
      double sampleTimes = global_sample_time;   // NOLINT

      if (test == 2) {
        testEpoch.precision = precision;      //	 NOLINT
        testEpoch.dmaTimes = dmaTimes;        // NOLINT
        testEpoch.sampleTimes = sampleTimes;  // NOLINT
      } else if (test == 0) {
        trainEpoch.precision = precision;      // NOLINT
        trainEpoch.dmaTimes = dmaTimes;        // NOLINT
        trainEpoch.sampleTimes = sampleTimes;  // NOLINT
      } else {
        afterTrainEpoch.precision = precision;      // NOLINT
        afterTrainEpoch.dmaTimes = dmaTimes;        // NOLINT
        afterTrainEpoch.sampleTimes = sampleTimes;  // NOLINT
      }
    }

    std::vector<EpochData> data;
    data.push_back(testEpoch);        // NOLINT
    data.push_back(afterTrainEpoch);  // NOLINT
    data.push_back(trainEpoch);       // NOLINT
    callback(py::cast(data));
  }
  double ending = get_timestamp();
  double total_time = ending - beggining;
  std::cout << "Finished experiment" << std::endl;
  std::cout << "Total time: " << total_time << std::endl;
  std::cout << "\n";
  return;
}

PYBIND11_MODULE(reckon_wb, m) {
  py::class_<EpochData>(m, "EpochData")
      .def(py::init<>())
      .def_readonly("number", &EpochData::number)
      .def_readonly("name", &EpochData::name)
      .def_readonly("precision", &EpochData::precision,
                    py::return_value_policy::copy)
      .def_readonly("sampleTimes", &EpochData::sampleTimes,
                    py::return_value_policy::copy)
      .def_readonly("dmaTimes", &EpochData::dmaTimes,
                    py::return_value_policy::copy);
  m.def("singleRun", &singleRun, "Call reckon training from python.",
        py::return_value_policy::copy);
  m.def("trainReckon", &trainReckon, "Get data of a single training session",
        py::return_value_policy::copy);
}
