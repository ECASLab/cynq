/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

// clang-format off
#include <algorithm>
#include <cstdint>
#include <cynq/accelerator.hpp>
#include <cynq/datamover.hpp>
#include <cynq/hardware.hpp>
#include <cynq/memory.hpp>
#include <iostream>
#include <memory>
#include <string>

#include <third-party/resources/kria-bitstreams/matmul/driver.h>
#include <third-party/timer.hpp>
// clang-format on

// HLS Types
#include "ap_fixed.h"

using DataT = ap_fixed<16, 6>;

typedef union {
  uint16_t rvalues[4];
  uint64_t packet;
} PacketU;

/**
 * @example zynq-mpsoc/xfcolor-detect.cpp
 * This is a sample use case for the CYNQ. It represents a
 * color detection algorithm from the XfOpenCV Library (now part of Vitis
 * Library)
 */

// Given by the example
static constexpr char kBitstream[] =
    "/home/ubuntu/docs/lleon/cynq-alveo/third-party/resources/kria-bitstreams"
    "/matmul/matmul.bit";

// Given by the design
static constexpr uint64_t kAccelAddress = EXAMPLE_KRIA_ACCEL_ADDR;
static constexpr uint64_t kDmaAddress = EXAMPLE_KRIA_DMA_ADDR;

int main(int argc, char** argv) {
  // NOTE: This is a basic example. Error checking has been removed to keep
  // simplicity but it is always recommended
  using namespace cynq;  // NOLINT
  INIT_PROFILER(cynq_profiler)

  if (argc != 4) {
    std::cerr << "ERROR: Cannot execute the example. Requires a parameter:"
              << std::endl
              << "\t matmul a_rows b_cols c_cols" << std::endl;
    return -1;
  }

  // Load image
  std::cout << "----- Loading image -----" << std::endl;
  // Get input size
  int a_rows = std::stoi(argv[1]);
  int b_cols = std::stoi(argv[2]);
  int c_cols = std::stoi(argv[3]);
  b_cols = b_cols < 8 ? 8 : (b_cols - (b_cols & 4));
  c_cols = c_cols < 8 ? 8 : (c_cols - (c_cols & 4));

  std::cout << "A rows: " << a_rows << "\n"
            << "B cols: " << b_cols << "\n"
            << "C cols: " << c_cols << std::endl;

  // Compute sizes
  int size_a = a_rows * b_cols;
  int size_b = c_cols * b_cols;
  int size_c = a_rows * c_cols;

  // Load hardware
  std::cout << "----- Initialising platform -----" << std::endl;
#ifdef PROFILE_MODE
  GET_PROFILE_INSTANCE(setup_time, cynq_profiler);
  setup_time->reset();
#endif
  std::shared_ptr<IHardware> platform =
      IHardware::Create(HardwareArchitecture::UltraScale, kBitstream);

  // Get an accelerator
  std::shared_ptr<IAccelerator> accel = platform->GetAccelerator(kAccelAddress);
  // Get a data mover
  std::shared_ptr<IDataMover> mover = platform->GetDataMover(kDmaAddress);
#ifdef PROFILE_MODE
  setup_time->tick();
#endif

  // Create buffers for input and output
  std::cout << "----- Creating memory -----" << std::endl;
  std::shared_ptr<IMemory> buf_mem_a = mover->GetBuffer(
      size_a * sizeof(uint16_t), accel->GetMemoryBank(0));
  std::shared_ptr<IMemory> buf_mem_b = mover->GetBuffer(
      size_b * sizeof(uint16_t), accel->GetMemoryBank(1));
  std::shared_ptr<IMemory> buf_mem_c = mover->GetBuffer(
      size_c * sizeof(uint16_t), accel->GetMemoryBank(2));

  std::cout << "----- Loading input -----" << std::endl;
  uint16_t* bo_a_map = buf_mem_a->HostAddress<uint16_t>().get();
  uint16_t* bo_b_map = buf_mem_b->HostAddress<uint16_t>().get();
  uint16_t* bo_c_map = buf_mem_c->HostAddress<uint16_t>().get();

  // Filling data
  //std::cout << "Filling Buffers\n";
  DataT as = 0.002, bs = 0.003;
  //std::cout << "A: " << std::endl;
  for (int elem = 0; elem < size_a; ++elem) {
    //std::cout << as << " ";
    bo_a_map[elem] = as.V;
    as += DataT{0.003};
    if ((elem + 1) % b_cols == 0) {
        std::cout << std::endl;
        as = 0.0025;
    }
  }
  //std::cout << "B: " << std::endl;
  for (int elem = 0; elem < size_b; ++elem) {
    //std::cout << bs << " ";
    bo_b_map[elem] = bs.V;
    bs += DataT{0.007};
    if ((elem + 1) % b_cols == 0) {
        //std::cout << std::endl;
        bs = 0.004;
    }
  }
  std::fill(bo_c_map, bo_c_map + size_c, 0);

  std::cout << "----- Configuring accelerator -----" << std::endl;
  accel->Write(XMATMUL_CONTROL_ADDR_A_ROWS_DATA, &a_rows, 1);
  accel->Write(XMATMUL_CONTROL_ADDR_B_COLS_DATA, &b_cols, 1);
  accel->Write(XMATMUL_CONTROL_ADDR_C_COLS_DATA, &c_cols, 1);
  accel->Attach(XMATMUL_CONTROL_ADDR_A_DATA, buf_mem_a);
  accel->Attach(XMATMUL_CONTROL_ADDR_B_DATA, buf_mem_b);
  accel->Attach(XMATMUL_CONTROL_ADDR_C_DATA, buf_mem_c);

#ifndef PROFILE_MODE
  std::cout << std::dec;
  std::cout << "----- Starting the Accelerator and Move Data -----"
            << std::endl;

  std::cout << "INFO: Trigger Upload" << std::endl;
  buf_mem_a->Sync(SyncType::HostToDevice);
  buf_mem_b->Sync(SyncType::HostToDevice);

  std::cout << "INFO: Starting Accelerator" << std::endl;
  accel->Start(StartMode::Once);
  accel->Sync();

  std::cout << "INFO: Trigger Download" << std::endl;
  buf_mem_c->Sync(SyncType::DeviceToHost);

  std::cout << "C: " << std::endl;
  for (int elem = 0; elem < size_c; ++elem) {
    DataT cs;
    cs.V = bo_c_map[elem];
    std::cout << cs << " ";
    if ((elem + 1) % c_cols == 0) std::cout << std::endl;
  }
#else
  START_PROFILE(kernel_execution, cynq_profiler, 1000)
  buf_mem_a->Sync(SyncType::HostToDevice);
  buf_mem_b->Sync(SyncType::HostToDevice);
  accel->Start(StartMode::Once);
  accel->Sync();
  buf_mem_c->Sync(SyncType::DeviceToHost);
  END_PROFILE(kernel_execution)
  std::cout << cynq_profiler << std::endl;
#endif

  // Stop the accel
  std::cout << "INFO: Stopping Accel" << std::endl;
  accel->Stop();
  return 0;
}
