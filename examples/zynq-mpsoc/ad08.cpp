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
#include <third-party/timer.hpp>
// clang-format on

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
 */

typedef union {
  uint16_t rvalues[4];
  uint64_t packet;
} PacketU;

using DataType = uint16_t;
using namespace cynq;  // NOLINT

// Given by the example
static constexpr char kBitstream[] = AD08_BITSTREAM_LOCATION;

// Given by the design
static constexpr uint64_t kElemWiseAddr = 0xA0000000;
static constexpr uint64_t kMatMulAddr = 0xA0020000;
static constexpr uint64_t kDmaAddress = 0xA0010000;
#define XMATMUL_CONTROL_ADDR_A_DATA 0x10
#define XMATMUL_CONTROL_ADDR_B_DATA 0x1c
#define XMATMUL_CONTROL_ADDR_C_DATA 0x28
#define XMATMUL_CONTROL_ADDR_A_ROWS_DATA 0x34
#define XMATMUL_CONTROL_ADDR_B_COLS_DATA 0x3c
#define XMATMUL_CONTROL_ADDR_C_COLS_DATA 0x44
#define XELEMENTWISE_CONTROL_ADDR_IN1_DATA 0x10
#define XELEMENTWISE_CONTROL_ADDR_IN2_DATA 0x1c
#define XELEMENTWISE_CONTROL_ADDR_OUT_R_DATA 0x28
#define XELEMENTWISE_CONTROL_ADDR_SIZE_DATA 0x34
#define XELEMENTWISE_CONTROL_ADDR_OP_DATA 0x3c

/* Auxiliary function to fill buffers */
static void fill_data(std::shared_ptr<IMemory> buffer, const size_t num,
                      const DataType start_value = 100,
                      const DataType step_value = 10) {
  DataType* data = buffer->HostAddress<DataType>().get();
  for (size_t i = 0; i < num; ++i) {
    data[i] = static_cast<DataType>(start_value + step_value * i);
  }
}

/* Auxiliary function to print */
static void print_data(std::shared_ptr<IMemory> buffer, const size_t num) {
  DataType* data = buffer->HostAddress<DataType>().get();
  for (size_t i = 0; i < num; ++i) {
    std::cout << data[i] << " ";
  }
  std::cout << std::endl;
}

int main(int argc, char** argv) {
  // NOTE: This is a basic example. Error checking has been removed to keep
  // simplicity but it is always recommended
  INIT_PROFILER(cynq_profiler)

  if (argc != 4) {
    std::cerr << "ERROR: Cannot execute the example. Requires a parameter:"
              << std::endl
              << "\t" << argv[0] << " a_rows b_cols c_cols" << std::endl;
    return -1;
  }

  // Load image
  std::cout << "----- Loading arguments -----" << std::endl;
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
  int op = 0;  // ADD

  // Load hardware
  std::cout << "----- Initialising platform -----" << std::endl;
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
#ifdef PROFILE_MODE
  setup_time->tick();
#endif

  // Create buffers for input and output
  std::cout << "----- Creating memory -----" << std::endl;
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

  std::cout << "----- Loading input -----" << std::endl;
  fill_data(buf_mem_mm_a, size_a, 1002);
  fill_data(buf_mem_mm_b, size_b, 55);
  fill_data(buf_mem_mm_c, size_c, 0, 0);
  fill_data(buf_mem_ew_a, size_a, 1002);
  fill_data(buf_mem_ew_b, size_b, 55);
  fill_data(buf_mem_ew_c, size_c, 0, 0);

  std::cout << "----- Configuring accelerators -----" << std::endl;
  matmul->Write(XMATMUL_CONTROL_ADDR_A_ROWS_DATA, &a_rows, 1);
  matmul->Write(XMATMUL_CONTROL_ADDR_B_COLS_DATA, &b_cols, 1);
  matmul->Write(XMATMUL_CONTROL_ADDR_C_COLS_DATA, &c_cols, 1);
  matmul->Attach(XMATMUL_CONTROL_ADDR_A_DATA, buf_mem_mm_a);
  matmul->Attach(XMATMUL_CONTROL_ADDR_B_DATA, buf_mem_mm_b);
  matmul->Attach(XMATMUL_CONTROL_ADDR_C_DATA, buf_mem_mm_c);

  elemwise->Write(XELEMENTWISE_CONTROL_ADDR_SIZE_DATA, &size_c, 1);
  elemwise->Write(XELEMENTWISE_CONTROL_ADDR_OP_DATA, &op, 1);
  elemwise->Attach(XELEMENTWISE_CONTROL_ADDR_IN1_DATA, buf_mem_ew_a);
  elemwise->Attach(XELEMENTWISE_CONTROL_ADDR_IN2_DATA, buf_mem_ew_b);
  elemwise->Attach(XELEMENTWISE_CONTROL_ADDR_OUT_R_DATA, buf_mem_ew_c);

  std::cout << std::dec;
  std::cout << "----- Starting the Accelerator and Move Data -----"
            << std::endl;

  std::cout << "INFO: Trigger Upload" << std::endl;
  buf_mem_mm_a->Sync(SyncType::HostToDevice);
  buf_mem_mm_b->Sync(SyncType::HostToDevice);
  buf_mem_ew_a->Sync(SyncType::HostToDevice);
  buf_mem_ew_b->Sync(SyncType::HostToDevice);

  std::cout << "INFO: Starting Accelerator: MatMul" << std::endl;
  matmul->Start(StartMode::Once);
  matmul->Sync();

  std::cout << "INFO: Trigger Download" << std::endl;
  buf_mem_mm_c->Sync(SyncType::DeviceToHost);

  std::cout << "INFO: Starting Accelerator: Element Wise" << std::endl;
  elemwise->Start(StartMode::Once);
  elemwise->Sync();

  std::cout << "INFO: Trigger Download" << std::endl;
  buf_mem_ew_c->Sync(SyncType::DeviceToHost);

  std::cout << "MatMul Result: " << std::endl;
  print_data(buf_mem_mm_c, size_c);
  std::cout << "ElementWise Result: " << std::endl;
  print_data(buf_mem_ew_c, size_c);

  return 0;
}
