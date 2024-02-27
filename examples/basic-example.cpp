/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */

#include <cstdint>
#include <cynq/accelerator.hpp>
#include <cynq/datamover.hpp>
#include <cynq/hardware.hpp>
#include <cynq/memory.hpp>
#include <iostream>
#include <memory>
#include <string>

/**
 * @example basic-example.cpp
 * This is a sample use case for the CYNQ. It represents a
 * matrix-multiplication from FAL
 */

#if !defined(EXAMPLE_BITSTREAM_LOCATION) || \
    !defined(EXAMPLE_DEFAULT_XCLBIN_LOCATION)
#error "Missing location macros for example"
#endif

// Given by the example
static constexpr char kBitstream[] = EXAMPLE_BITSTREAM_LOCATION;
static constexpr char kXclBin[] = EXAMPLE_DEFAULT_XCLBIN_LOCATION;

// Given by the design
static constexpr uint64_t kAccelAddress = EXAMPLE_ACCEL_ADDR;
static constexpr uint64_t kDmaAddress = EXAMPLE_DMA_ADDR;
static constexpr uint64_t kAddrWriteInputCols = 24;
static constexpr uint64_t kAddrWriteOutputCols = 40;
static constexpr uint64_t kAddrReadInputCols = 32;
static constexpr uint64_t kAddrReadOutputCols = 48;

// Data parameters
using DataType = uint16_t;
static constexpr int input_a_cols = 400;
static constexpr int input_a_rows = 2;
static constexpr int input_b_cols = 4;
static constexpr int input_b_rows = input_a_cols;
static constexpr int output_cols = input_b_cols;
static constexpr int output_rows = input_a_rows;
static constexpr int word_size = sizeof(DataType);

// Fill data
void FillData(DataType* A, DataType* B, DataType* C) {
  for (uint32_t row = 0; row < input_b_cols; ++row) {
    for (uint32_t col = 0; col < input_a_cols; ++col) {
      A[((row % input_a_rows) * input_a_cols) + (col % input_a_cols)] =
          row * col;
      B[((col % input_b_rows) * input_b_cols) + (row % input_b_cols)] =
          row * col;
      C[((row % output_rows) * output_cols) + (col % output_cols)] = 0;
    }
  }
}

// Print results
void PrintData(DataType* C) {
  std::cout << "Output: " << std::endl;
  for (int i = 0; i < output_rows; ++i) {
    for (int j = 0; j < output_cols; ++j) {
      std::cout << C[i * output_cols + j] << " ";
    }
    std::cout << std::endl;
  }
}

int main() {
  // NOTE: This is a basic example. Error checking has been removed to keep
  // simplicity but it is always recommended
  using namespace cynq;  // NOLINT

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
  DataType* A = in_mem->HostAddress<DataType>().get();
  DataType* B = A + (input_a_cols * input_a_rows);
  DataType* C = out_mem->HostAddress<DataType>().get();

  // Fill the data
  FillData(A, B, C);

  // Start the accel
  accel->Start(StartMode::Continuous);
  // Check the control register
  auto devstatus = accel->GetStatus();
  std::cout << "\tAccel Status: " << static_cast<int>(devstatus) << std::endl;

  std::cout << "----- Configuring accelerator -----" << std::endl;
  // Configure params
  accel->Write(kAddrWriteInputCols, &input_a_cols, 1);
  accel->Write(kAddrWriteOutputCols, &output_cols, 1);
  // Check params
  int res_input_a_cols = 0;
  int res_output_cols = 0;
  accel->Read(kAddrReadInputCols, &res_input_a_cols, 1);
  accel->Read(kAddrReadOutputCols, &res_output_cols, 1);

  std::cout << "----- Moving the data -----" << std::endl;
  // Move the data
  mover->Upload(in_mem, in_mem->Size(), 0, ExecutionType::Async);
  mover->Download(out_mem, out_mem->Size(), 0, ExecutionType::Sync);

  // Stop the accel
  accel->Stop();

  // Read the output on *out_data*
  PrintData(C);

  return 0;
}
