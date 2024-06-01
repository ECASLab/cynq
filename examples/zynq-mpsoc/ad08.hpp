/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#pragma once

#include <cstdint>
#include <cynq/memory.hpp>
#include <iostream>
#include <memory>
#include <string>

// Given by the example
static constexpr char kBitstream[] = AD08_BITSTREAM_LOCATION;

// Given by the design
// The Addressed are given by the Vivado design when creating the block diagram
// You can find them in the Address Editor
static constexpr uint64_t kElemWiseAddr = 0xA0000000;
static constexpr uint64_t kMatMulAddr = 0xA0020000;
static constexpr uint64_t kDmaAddress = 0xA0010000;
// These are the registers for the matmul IP. They can be found in the drivers
// After the export of the HLS code
#define XMATMUL_CONTROL_ADDR_A_DATA 0x10
#define XMATMUL_CONTROL_ADDR_B_DATA 0x1c
#define XMATMUL_CONTROL_ADDR_C_DATA 0x28
#define XMATMUL_CONTROL_ADDR_A_ROWS_DATA 0x34
#define XMATMUL_CONTROL_ADDR_B_COLS_DATA 0x3c
#define XMATMUL_CONTROL_ADDR_C_COLS_DATA 0x44
// These are the registers for the elementwise IP. They can be found in the
// drivers. After the export of the HLS code
#define XELEMENTWISE_CONTROL_ADDR_IN1_DATA 0x10
#define XELEMENTWISE_CONTROL_ADDR_IN2_DATA 0x1c
#define XELEMENTWISE_CONTROL_ADDR_OUT_R_DATA 0x28
#define XELEMENTWISE_CONTROL_ADDR_SIZE_DATA 0x34
#define XELEMENTWISE_CONTROL_ADDR_OP_DATA 0x3c

// Datatype of the accelerator. Defined when coding it
using DataType = uint16_t;

#define AD08_INFO(msg) std::cout << "[INFO]: " << (msg) << std::endl;

#ifdef PROFILE_MODE
#undef AD08_INFO
#define AD08_INFO(msg) AD08_UNUSED(msg);
void AD08_UNUSED(const std::string&) {}
#endif

/* Auxiliary function to fill buffers */
static void fill_data(std::shared_ptr<cynq::IMemory> buffer, const size_t num,
                      const DataType start_value = 100,
                      const DataType step_value = 10) {
  DataType* data = buffer->HostAddress<DataType>().get();
  for (size_t i = 0; i < num; ++i) {
    data[i] = static_cast<DataType>(start_value + step_value * i);
  }
}

#ifndef PROFILE_MODE
/* Auxiliary function to print */
static void print_data(std::shared_ptr<cynq::IMemory> buffer,
                       const size_t num) {
  DataType* data = buffer->HostAddress<DataType>().get();
  for (size_t i = 0; i < num; ++i) {
    std::cout << data[i] << " ";
  }
  std::cout << std::endl;
}
#endif
