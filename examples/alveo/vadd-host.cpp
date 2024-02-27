/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#include <algorithm>
#include <cstring>
#include <cynq/accelerator.hpp>
#include <cynq/datamover.hpp>
#include <cynq/hardware.hpp>
#include <cynq/memory.hpp>
#include <iostream>
#include <string>

/**
 * @example vadd-host.cpp
 * This is a sample of how the hello world xrt example looks like
 * in CYNQ. It performs two vector additions.
 */

#if !defined(EXAMPLE_ALVEO_VADD_XCLBIN_LOCATION)
#error "Missing location macros for example"
#endif

// Given by the example
static constexpr char kDefaultXclBin[] = EXAMPLE_ALVEO_VADD_XCLBIN_LOCATION;
static constexpr int kDataSize = 4096;

int main(int argc, char **argv) {
  // NOTE: This is a basic example. Error checking has been removed to keep
  // simplicity but it is always recommended
  using namespace cynq;  // NOLINT
  uint datasize = kDataSize;

  std::string xclbin_path;

  // Load argument
  xclbin_path = argc > 1 ? std::string(argv[1]) : kDefaultXclBin;

  // Create the platform
  std::cout << "----- Initialising platform -----" << std::endl;
  std::shared_ptr<IHardware> platform =
      IHardware::Create(HardwareArchitecture::Alveo, "", xclbin_path);

  // Get an accelerator
  std::shared_ptr<IAccelerator> accel = platform->GetAccelerator("vadd");
  // Get a data mover
  std::shared_ptr<IDataMover> mover = platform->GetDataMover(0);

  // Create buffers for input and output
  std::cout << "----- Creating memory -----" << std::endl;
  std::size_t vec_size = sizeof(int) * kDataSize;
  std::shared_ptr<IMemory> bo_0 =
      mover->GetBuffer(vec_size, accel->GetMemoryBank(0));
  std::shared_ptr<IMemory> bo_1 =
      mover->GetBuffer(vec_size, accel->GetMemoryBank(1));
  std::shared_ptr<IMemory> bo_out =
      mover->GetBuffer(vec_size, accel->GetMemoryBank(2));

  // Get the host pointers for input/outut
  auto bo_0_map = bo_0->HostAddress<int>().get();
  auto bo_1_map = bo_1->HostAddress<int>().get();
  auto bo_out_map = bo_out->HostAddress<int>().get();

  std::cout << "----- Loading input -----" << std::endl;
  std::cout << "Allocate Buffer in Global Memory\n";
  std::fill(bo_0_map, bo_0_map + kDataSize, 0);
  std::fill(bo_1_map, bo_1_map + kDataSize, 0);
  std::fill(bo_out_map, bo_out_map + kDataSize, 0);

  // Create the test data
  std::cout << "----- Create reference data -----" << std::endl;
  int bufReference[kDataSize];
  for (uint i = 0; i < kDataSize; ++i) {
    bo_0_map[i] = i;
    bo_1_map[i] = i;
    bufReference[i] = bo_0_map[i] + bo_1_map[i];
  }

  std::cout << "----- Moving the data -----" << std::endl;
  // Move the data
  std::cout << "synchronize input buffer data to device global memory\n";
  mover->Upload(bo_0, bo_0->Size(), 0, ExecutionType::Async);
  mover->Upload(bo_1, bo_1->Size(), 0, ExecutionType::Async);

  std::cout << "----- Configuring accelerator -----" << std::endl;
  auto devstatus = accel->GetStatus();
  std::cout << "\tAccel Status: " << static_cast<int>(devstatus) << std::endl;
  uint64_t bo_0_addr = (uint64_t)bo_0->DeviceAddress<uint32_t>().get();
  uint64_t bo_1_addr = (uint64_t)bo_1->DeviceAddress<uint32_t>().get();
  uint64_t bo_out_addr = (uint64_t)bo_out->DeviceAddress<uint32_t>().get();
  accel->Attach(0, &bo_0_addr);
  accel->Attach(1, &bo_1_addr);
  accel->Attach(2, &bo_out_addr);
  accel->Attach(3, &datasize);

  accel->Start(StartMode::Once);
  accel->Sync();

  std::cout << "----- Moving the data back -----" << std::endl;
  mover->Download(bo_out, bo_out->Size(), 0, ExecutionType::Sync);

  std::cout << "----- Validating -----" << std::endl;
  // Validate our results
  if (std::memcmp(bo_out_map, bufReference, kDataSize)) {
    std::cerr << "Value read back does not match reference" << std::endl;
    return -1;
  }

  std::cout << "Test passed" << std::endl;
  return 0;
}
