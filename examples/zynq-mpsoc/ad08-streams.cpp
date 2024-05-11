/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

// clang-format off
#include "ad08.hpp" // NOLINT

#include <algorithm>
#include <cstdint>
#include <cynq/accelerator.hpp>
#include <cynq/datamover.hpp>
#include <cynq/execution-graph.hpp>
#include <cynq/hardware.hpp>
#include <cynq/memory.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <third-party/timer.hpp>
// clang-format on

/**
 * @example zynq-mpsoc/ad08-streams.cpp
 *
 * This is a use case that involves two accelerators with AXI-MM ports.
 * The accelerators are executed in parallel using IExecutionGraph
 */

/*
 * Running: sudo ./builddir/examples/ad08-kria 4 4 4
 *
 * Result:
 *   MatMul Result:
 *   728 1379 2026 2674 3322 3970 4618 5267 787 1483 2181 2879 3578 4276 4975
 *   5672 842 1589 2338 3087 3835 4583 5330 6078 898 1696 2495 3292 4091 4888
 *   5686 6486
 *   ElementWise Result:
 *   1057 1077 1097 1117 1137 1157 1177 1197 1217 1237 1257 1277 1297 1317
 *   1337 1357 1377 1397 1417 1437 1457 1477 1497 1517 1537 1557 1577 1597
 *   1617 1637 1657 1677
 *
 * The matmul: C = A x B
 * The elementwise: C = A + B
 */

using namespace cynq;  // NOLINT

int main(int argc, char** argv) {
  // NOTE: This is a basic example. Error checking has been removed to keep
  // simplicity but it is always recommended
  INIT_PROFILER(cynq_profiler)
#ifdef PROFILE_MODE
  GET_PROFILE_INSTANCE(total_time, cynq_profiler);
  total_time->reset();
#endif

  if (argc != 4) {
    std::cerr << "ERROR: Cannot execute the example. Requires a parameter:"
              << std::endl
              << "\t" << argv[0] << " a_rows b_cols c_cols" << std::endl;
    return -1;
  }

  // Load image
  AD08_INFO("Loading arguments");
  // Get input size
  int a_rows = std::stoi(argv[1]);
  int b_cols = std::stoi(argv[2]);
  int c_cols = std::stoi(argv[3]);
  b_cols = b_cols < 8 ? 8 : (b_cols - (b_cols & 4));
  c_cols = c_cols < 8 ? 8 : (c_cols - (c_cols & 4));

  AD08_INFO(std::string(" A rows: ") + std::to_string(a_rows));
  AD08_INFO(std::string(" B cols: ") + std::to_string(b_cols));
  AD08_INFO(std::string(" C cols: ") + std::to_string(c_cols));

  // Compute sizes
  int size_a = a_rows * b_cols;
  int size_b = c_cols * b_cols;
  int size_c = a_rows * c_cols;
  int op = 0;  // ADD

  // Load hardware
  AD08_INFO("Initialising platform");
#ifdef PROFILE_MODE
  GET_PROFILE_INSTANCE(setup_time, cynq_profiler);
  setup_time->reset();
#endif
  std::shared_ptr<IHardware> platform =
      IHardware::Create(HardwareArchitecture::UltraScale, kBitstream);

  // Get an accelerator
  std::shared_ptr<IAccelerator> matmul = platform->GetAccelerator(kMatMulAddr);
  std::shared_ptr<IAccelerator> elemwise =
      platform->GetAccelerator(kElemWiseAddr);
  // Get a data mover
  std::shared_ptr<IDataMover> mover = platform->GetDataMover(kDmaAddress);
  // Get two streams
  std::shared_ptr<IExecutionGraph> matmul_stream =
      platform->GetExecutionStream("matmul");
  std::shared_ptr<IExecutionGraph> elemwise_stream =
      platform->GetExecutionStream("elemwise");
#ifdef PROFILE_MODE
  setup_time->tick();
#endif

  // Create buffers for input and output
  AD08_INFO("Creating memory");
  std::shared_ptr<IMemory> buf_mem_mm_a =
      mover->GetBuffer(size_a * sizeof(DataType), matmul->GetMemoryBank(0));
  std::shared_ptr<IMemory> buf_mem_mm_b =
      mover->GetBuffer(size_b * sizeof(DataType), matmul->GetMemoryBank(1));
  std::shared_ptr<IMemory> buf_mem_mm_c =
      mover->GetBuffer(size_c * sizeof(DataType), matmul->GetMemoryBank(2));
  std::shared_ptr<IMemory> buf_mem_ew_a =
      mover->GetBuffer(size_c * sizeof(DataType), elemwise->GetMemoryBank(0));
  std::shared_ptr<IMemory> buf_mem_ew_b =
      mover->GetBuffer(size_c * sizeof(DataType), elemwise->GetMemoryBank(1));
  std::shared_ptr<IMemory> buf_mem_ew_c =
      mover->GetBuffer(size_c * sizeof(DataType), elemwise->GetMemoryBank(2));

  AD08_INFO("Loading input");
  fill_data(buf_mem_mm_a, size_a, 1002);
  fill_data(buf_mem_mm_b, size_b, 55);
  fill_data(buf_mem_mm_c, size_c, 0, 0);
  fill_data(buf_mem_ew_a, size_a, 1002);
  fill_data(buf_mem_ew_b, size_b, 55);
  fill_data(buf_mem_ew_c, size_c, 0, 0);

  AD08_INFO("Configuring accelerators");
#ifdef PROFILE_MODE
  GET_PROFILE_INSTANCE(configuration_time, cynq_profiler);
  configuration_time->reset();
#endif
  matmul->Write(matmul_stream, XMATMUL_CONTROL_ADDR_A_ROWS_DATA, &a_rows, 1);
  matmul->Write(matmul_stream, XMATMUL_CONTROL_ADDR_B_COLS_DATA, &b_cols, 1);
  matmul->Write(matmul_stream, XMATMUL_CONTROL_ADDR_C_COLS_DATA, &c_cols, 1);
  matmul->Attach(XMATMUL_CONTROL_ADDR_A_DATA, buf_mem_mm_a);
  matmul->Attach(XMATMUL_CONTROL_ADDR_B_DATA, buf_mem_mm_b);
  matmul->Attach(XMATMUL_CONTROL_ADDR_C_DATA, buf_mem_mm_c);

  elemwise->Write(elemwise_stream, XELEMENTWISE_CONTROL_ADDR_SIZE_DATA, &size_c,
                  1);
  elemwise->Write(elemwise_stream, XELEMENTWISE_CONTROL_ADDR_OP_DATA, &op, 1);
  elemwise->Attach(XELEMENTWISE_CONTROL_ADDR_IN1_DATA, buf_mem_ew_a);
  elemwise->Attach(XELEMENTWISE_CONTROL_ADDR_IN2_DATA, buf_mem_ew_b);
  elemwise->Attach(XELEMENTWISE_CONTROL_ADDR_OUT_R_DATA, buf_mem_ew_c);
#ifdef PROFILE_MODE
  configuration_time->tick();
#endif

  AD08_INFO("Starting the Accelerator and Move Data");
  AD08_INFO("Trigger Upload");
#ifdef PROFILE_MODE
  GET_PROFILE_INSTANCE(upload_time, cynq_profiler);
  upload_time->reset();
#endif
  buf_mem_mm_a->Sync(matmul_stream, SyncType::HostToDevice);
  buf_mem_mm_b->Sync(matmul_stream, SyncType::HostToDevice);
  buf_mem_ew_a->Sync(elemwise_stream, SyncType::HostToDevice);
  buf_mem_ew_b->Sync(elemwise_stream, SyncType::HostToDevice);
#ifdef PROFILE_MODE
  upload_time->tick();
#endif

  AD08_INFO("Starting Accelerators");
#ifdef PROFILE_MODE
  GET_PROFILE_INSTANCE(compute_time, cynq_profiler);
  compute_time->reset();
#endif
  matmul->Start(matmul_stream, StartMode::Once);
  matmul->Sync(matmul_stream);
  elemwise->Start(elemwise_stream, StartMode::Once);
  elemwise->Sync(elemwise_stream);
#ifdef PROFILE_MODE
  compute_time->tick();
#endif

  AD08_INFO("Trigger Download");
#ifdef PROFILE_MODE
  GET_PROFILE_INSTANCE(download_time, cynq_profiler);
  download_time->reset();
#endif
  buf_mem_mm_c->Sync(matmul_stream, SyncType::DeviceToHost);
  buf_mem_ew_c->Sync(elemwise_stream, SyncType::DeviceToHost);
#ifdef PROFILE_MODE
  download_time->tick();
#endif

  AD08_INFO("Synchronise streams");
#ifdef PROFILE_MODE
  GET_PROFILE_INSTANCE(sync_time, cynq_profiler);
  sync_time->reset();
#endif
  matmul_stream->Sync();
  elemwise_stream->Sync();
#ifdef PROFILE_MODE
  sync_time->tick();
#endif

#ifndef PROFILE_MODE
  std::cout << "MatMul Result: " << std::endl;
  print_data(buf_mem_mm_c, size_c);
  std::cout << "ElementWise Result: " << std::endl;
  print_data(buf_mem_ew_c, size_c);
#endif

#ifdef PROFILE_MODE
  total_time->tick();
#endif
  std::cout << cynq_profiler << std::endl;

  return 0;
}
