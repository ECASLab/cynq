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

// Addresses of the accelerator
static constexpr uint64_t kAccelNumDataAddr = 0x20;
static constexpr uint kNumData = 64;

int main() {
  using namespace cynq;  // NOLINT

  const int input_size = kNumData * sizeof(float);
  const int output_size = kNumData * sizeof(float);

  // Create the platform
  std::shared_ptr<IHardware> platform =
      IHardware::Create(HardwareArchitecture::UltraScale, kBitstream, kXclBin);

  // Get an accelerator
  std::shared_ptr<IAccelerator> accel = platform->GetAccelerator(kAccelAddress);
  // Get a data mover
  std::shared_ptr<IDataMover> mover = platform->GetDataMover(kDmaAddress);

  // Create buffers for input and output
  std::shared_ptr<IMemory> in_mem =
      mover->GetBuffer(input_size, MemoryType::Dual);
  std::shared_ptr<IMemory> out_mem =
      mover->GetBuffer(output_size, MemoryType::Dual);

  // Get the host pointers for input/outut
  std::shared_ptr<float> in_data = in_mem->HostAddress<float>();
  std::shared_ptr<float> out_data = out_mem->HostAddress<float>();

  // Fill data on *in_data*...

  // Start the accel in autorestart
  accel->Start(StartMode::Continuous);

  // Read the defaults:
  uint32_t incols = 0;
  uint32_t outcols = 0;

  accel->Read(32, &incols, 1);
  accel->Read(48, &outcols, 1);
  std::cout << "Initial InCols: " << incols << std::endl;
  std::cout << "Initial OutCols: " << outcols << std::endl;

  // Configure the accel
  incols = kNumData;
  outcols = kNumData;
  accel->Write(24, &incols, 1);
  accel->Write(40, &outcols, 1);

  accel->Read(32, &incols, 1);
  accel->Read(48, &outcols, 1);
  std::cout << "Configured InCols: " << incols << std::endl;
  std::cout << "Configured OutCols: " << outcols << std::endl;

  // Move the data
  in_mem->Sync(SyncType::HostToDevice);
  mover->Upload(in_mem, in_mem->Size(), ExecutionType::Sync);
  mover->Download(out_mem, out_mem->Size(), ExecutionType::Sync);

  // Stop the accel
  accel->Stop();

  // Read the output on *out_data*
  return 0;
}
