/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

// cynq headers
#include <cynq/accelerator.hpp>
#include <cynq/datamover.hpp>
#include <cynq/hardware.hpp>
#include <cynq/memory.hpp>

#if !defined(EXAMPLE_BITSTREAM_LOCATION) || \
    !defined(EXAMPLE_DEFAULT_XCLBIN_LOCATION)
#error "Missing location macros for example"
#endif

static constexpr char kBitstream[] = EXAMPLE_BITSTREAM_LOCATION;
static constexpr char kXclBin[] = EXAMPLE_DEFAULT_XCLBIN_LOCATION;

// Given by the design
static constexpr uint64_t kAccelAddress = EXAMPLE_ACCEL_ADDR;
static constexpr uint64_t kDmaAddress = EXAMPLE_DMA_ADDR;

using DataType = uint16_t;

int main() {
  using namespace cynq;  // NOLINT

  const int input_a_cols = 400;
  const int input_a_rows = 2;
  const int input_b_cols = 4;
  const int input_b_rows = input_a_cols;
  const int output_cols = input_b_cols;
  const int output_rows = input_a_rows;
  const int word_size = sizeof(DataType);

  const size_t input_size =
      (input_a_cols * input_a_rows + input_b_cols * input_b_rows) * word_size;
  const size_t output_size = output_cols * output_rows * word_size;

  // Create the platform
  std::cout << "----- Initialising platform -----" << std::endl;
  std::shared_ptr<IHardware> platform =
      IHardware::Create(HardwareArchitecture::UltraScale, kBitstream, kXclBin);

  // Get an accelerator
  std::shared_ptr<IAccelerator> accel = platform->GetAccelerator(kAccelAddress);
  // Get a data mover
  std::shared_ptr<IDataMover> mover = platform->GetDataMover(kDmaAddress);

  // Create buffers for input and output
  std::cout << "----- Creating memory -----" << std::endl;
  std::shared_ptr<IMemory> in_mem = mover->GetBuffer(input_size);
  std::shared_ptr<IMemory> out_mem = mover->GetBuffer(output_size);

  // Get the host pointers for input/outut
  std::cout << "----- Loading input -----" << std::endl;
  std::shared_ptr<DataType> in_data = in_mem->HostAddress<DataType>();
  std::shared_ptr<DataType> out_data = out_mem->HostAddress<DataType>();
  DataType* A = in_data.get();
  DataType* B = A + (input_a_cols * input_a_rows);
  DataType* C = out_data.get();

  // Fill the data
  for (uint32_t row = 0; row < input_b_cols; ++row) {
    for (uint32_t col = 0; col < input_a_cols; ++col) {
      A[((row % input_a_rows) * input_a_cols) + (col % input_a_cols)] =
          row * col;
      B[((col % input_b_rows) * input_b_cols) + (row % input_b_cols)] =
          row * col;
      C[((row % output_rows) * output_cols) + (col % output_cols)] = 0;
    }
  }

  // Synchronise data buffer
  std::cout << "----- Synchronise Input -----" << std::endl;
  in_mem->Sync(SyncType::HostToDevice);

  // Start the accel
  accel->Start(StartMode::Continuous);
  // Check the control register
  auto devstatus = accel->GetStatus();
  std::cout << "\tAccel Status: " << static_cast<int>(devstatus) << std::endl;

  std::cout << "----- Configuring accelerator -----" << std::endl;
  // Configure params
  accel->Write(24, &input_a_cols, 1);
  accel->Write(40, &output_cols, 1);
  // Check params
  int res_input_a_cols = 0;
  int res_output_cols = 0;
  accel->Read(32, &res_input_a_cols, 1);
  accel->Read(48, &res_output_cols, 1);
  std::cout << "Configured A columns: " << input_a_cols
            << " Resulting: " << res_input_a_cols << std::endl;
  std::cout << "Configured C columns: " << output_cols
            << " Resulting: " << res_output_cols << std::endl;

  std::cout << "----- Moving the data -----" << std::endl;
  // Move the data
  mover->Upload(in_mem, in_mem->Size(), 0, ExecutionType::Sync);
  mover->Download(out_mem, out_mem->Size(), 0, ExecutionType::Sync);

  // Stop the accel
  accel->Stop();

  std::cout << "----- Synchronising output -----" << std::endl;
  out_mem->Sync(SyncType::DeviceToHost);

  // Read the output on *out_data*
  std::cout << "Output: " << std::endl;
  for (int i = 0; i < output_rows; ++i) {
    for (int j = 0; j < output_cols; ++j) {
      std::cout << C[i * output_cols + j] << " ";
    }
    std::cout << std::endl;
  }

  return 0;
}
