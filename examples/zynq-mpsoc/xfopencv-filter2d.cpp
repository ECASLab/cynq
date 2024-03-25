/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

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

#include <third-party/stb/stb_image.h>
#include <third-party/stb/stb_image_write.h>
#include <third-party/resources/kria-bitstreams/xfopencv-filter2d/driver.h>
#include <third-party/timer.hpp>
// clang-format on

/**
 * @example zynq-mpsoc/xfopencv-filter2d.cpp
 * This is a sample use case for the CYNQ. It represents a
 * filter 2D from the XfOpenCV Library (now part of Vitis Library)
 */

#ifndef XFOPENCV_FILTER2D_BITSTREAM_LOCATION
#error "Missing location macros for example"
#endif

// Given by the example
static constexpr char kBitstream[] = XFOPENCV_FILTER2D_BITSTREAM_LOCATION;

// Given by the design
static constexpr uint64_t kAccelAddress = EXAMPLE_KRIA_ACCEL_ADDR;
static constexpr uint64_t kDmaAddress = EXAMPLE_KRIA_DMA_ADDR;
static constexpr uint64_t kWidthAddress = XKRNL_FILTER2D_CONTROL_ADDR_WIDTH_DATA;
static constexpr uint64_t kHeightAddress = XKRNL_FILTER2D_CONTROL_ADDR_HEIGHT_DATA;

int main(int argc, char** argv) {
  // NOTE: This is a basic example. Error checking has been removed to keep
  // simplicity but it is always recommended
  using namespace cynq;  // NOLINT
  INIT_PROFILER(cynq_profiler)

  if (argc != 2) {
    std::cerr << "ERROR: Cannot execute the example. Requires a parameter:"
              << std::endl
              << "\t xfopencv-filter2d <IMAGE_PATH.png>" << std::endl;
    return -1;
  }

  // Load image
  std::cout << "----- Loading image -----" << std::endl;
  int width, height, channels;
  uint8_t* img = stbi_load(argv[1], &width, &height, &channels, 1);
  if (!img) {
    std::cerr << "ERROR: Cannot load the image: " << argv[1] << std::endl;
    return -1;
  }

  std::cout << "INFO: Loaded image " << argv[1] << " of size " << width << "x"
            << height << " and " << channels << " channels"
            << " (only 1 is taken)" << std::endl;

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
  const size_t img_size = width * height;
  std::cout << "INFO: Image size: " << img_size << " bytes" << std::endl;
  std::shared_ptr<IMemory> in_mem = mover->GetBuffer(img_size);
  std::shared_ptr<IMemory> out_mem = mover->GetBuffer(img_size);

  std::cout << "----- Loading input -----" << std::endl;
  uint8_t* in_ptr = in_mem->HostAddress<uint8_t>().get();
  uint8_t* out_ptr = out_mem->HostAddress<uint8_t>().get();
  std::copy(img, img + img_size, in_ptr);

  std::cout << "----- Configuring accelerator -----" << std::endl;
  accel->Write(kWidthAddress, &width, 1);
  accel->Write(kHeightAddress, &height, 1);
  accel->Attach(XKRNL_FILTER2D_CONTROL_ADDR_IN_R_DATA, in_mem);
  accel->Attach(XKRNL_FILTER2D_CONTROL_ADDR_OUT_R_DATA, out_mem);

  //accel->Start(StartMode::Continuous);
#ifndef PROFILE_MODE
  std::cout << std::dec;
  std::cout << "----- Starting the Accelerator and Move Data -----"
            << std::endl;

  std::cout << "INFO: Trigger Upload: " << img_size << " bytes" << std::endl;
  mover->Upload(in_mem, img_size, 0, ExecutionType::Async);

  std::cout << "INFO: Trigger Download " << img_size << " bytes" << std::endl;
  mover->Download(out_mem, img_size, 0, ExecutionType::Sync);
#else
  START_PROFILE(kernel_execution, cynq_profiler, 100)
  in_mem->Sync(SyncType::HostToDevice);
  accel->Start(StartMode::Once);
  accel->Sync();
  out_mem->Sync(SyncType::DeviceToHost);
  END_PROFILE(kernel_execution)
  std::cout << cynq_profiler << std::endl;
#endif
  // Stop the accel
  std::cout << "INFO: Stopping Accel" << std::endl;
  accel->Stop();

  // Save the result
  std::cout << "----- Saving resulting image -----" << std::endl;
  stbi_write_png("result.png", width, height, 1, out_ptr, width);

  stbi_image_free(img);
  img = nullptr;

  return 0;
}
