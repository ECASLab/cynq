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
#include <memory>
#include <string>

static constexpr char kBitstream[] = "./overlay.bit";
static constexpr char kXclBin[] = "./default.xclbin";

// Given by the design
static constexpr uint64_t kAccelAddress = 0xa0000000;
static constexpr uint64_t kDmaAddress = 0xa0010000;

// Addresses of the accelerator
static constexpr uint64_t kAccelNumDataAddr = 0x20;
static constexpr uint kNumData = 64;

// All structures developed in cynq must be in cynq namespace
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

  // Configure the accel
  accel->Write(kAccelNumDataAddr, &kNumData, kNumData);

  // Start the accel in autorestart
  accel->Start(StartMode::Continuous);

  // Move the data
  in_mem->Sync(SyncType::HostToDevice);
  mover->Upload(in_mem, in_mem->Size(), ExecutionType::Sync);
  mover->Download(out_mem, out_mem->Size(), ExecutionType::Sync);

  // Stop the accel
  accel->Stop();

  // Read the output on *out_data*
  return 0;
}
