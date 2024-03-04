# CYNQ

Framework to develop FPGA applications in C++ with the easiness of PYNQ

## Introduction

CYNQ is a C++ framework to implement FPGA-based accelerated applications with the same ease of use as PYNQ framework for Python. This allows users to implement their own applications with better performance than in Python and avoids the long processing times of coding applications with Vitis.

## Dependencies

1. Meson >= 1.x
2. Python >= 3.8
3. GCC >= 9.x
4. XRT >= 2.13
5. Linux FPGA Manager

## Index

* [Foundations](docs/Foundations.md)
  * [Class Diagram](docs/ClassDiagram.md)
* [Installation](docs/Installation.md)
* [Getting Started](docs/GettingStarted.md)
* [About](docs/About.md)

## How does CYNQ look like?

CYNQ is pretty similar to PYNQ, let's have a look.

PYNQ:

```python
from pynq import allocate, Overlay

# Configure the FPGA
design = Overlay("design.bit")

# Extract the accelerator (IP Core) and DMA
dma = design.axi_dma_0
accel = design.multiplication_accel_0

# Allocate buffers
inbuf = allocate(shape=(input_elements,), dtype=np.uint16)
outbuf = allocate(shape=(output_elements,), dtype=np.uint16)

# Run
dma.sendchannel.transfer(inbuf)
accel.write(accel.register_map.CTRL.address, 0x81)
accel.write(accel.register_map.n_elements.address, input_elements)
dma.recvchannel.transfer(outbuf)
dma.recvchannel.wait()

# Dispose the buffers
del input_hw
del output_hw
```

With CYNQ for Xilinx Ultrascale+:

```c++
#include <cynq/cynq.hpp>

using namespace cynq;

// Configure the FPGA
auto kArch = HardwareArchitecture::UltraScale;
auto platform = IHardware::Create(kArch, "design.bit");

// Extract the accelerator (IP Core) and DMA
// Addresses are given by the design
const uint64_t accel_addr = 0xa000000;
const uint64_t dma_addr = 0xa0010000;
auto accel = platform->GetAccelerator(accel_addr);
auto dma = platform->GetDataMover(dma_addr);

// Allocate buffers and get the pointers
auto inbuf = mover->GetBuffer(input_size, accel->GetMemoryBank(0));
auto outbuf = mover->GetBuffer(output_size, accel->GetMemoryBank(1));
uint16_t* input_ptr = inbuf->HostAddress<uint16_t>().get();
uint16_t* output_ptr = outbuf->HostAddress<uint16_t>().get();

// Configure data - Bus: AXI4 Stream is handled by DMA
const uint32_t num_elements = 4096;
const uint64_t addr_num_elements = 0x20;
accel->Write(addr_num_elements, num_elements);

// Run
mover->Upload(in_mem, infbuf->Size(), 0, ExecutionType::Async);
accel->Start(StartMode::Continuous);
mover->Download(out_mem, outbuf->Size(), 0, ExecutionType::Sync);
accel->Stop();

// Dispose? We use RAII
```

With CYNQ for Alveo

```c++
#include <cynq/cynq.hpp>

using namespace cynq;

// Configure the FPGA
auto kArch = HardwareArchitecture::Alveo;
auto platform = IHardware::Create(kArch, "design.xclbin");

// Extract the accelerator (IP Core) and DMA
// Addresses are given by the design
auto accel = platform->GetAccelerator("vadd");
auto dma = platform->GetDataMover(0);

// Allocate buffers and get the pointers
auto inbuf = mover->GetBuffer(input_size, accel->GetMemoryBank(0));
auto outbuf = mover->GetBuffer(output_size, accel->GetMemoryBank(1));
uint16_t* input_ptr = inbuf->HostAddress<uint16_t>().get();
uint16_t* output_ptr = outbuf->HostAddress<uint16_t>().get();

// Configure the accel - memory mapped
const uint32_t num_elements = 4096;
accel->Attach(0, bo_0);
accel->Attach(1, bo_1);
accel->Attach(2, &num_elements);

// Run
mover->Upload(in_mem, infbuf->Size(), 0, ExecutionType::Async);
accel->Start(StartMode::Once);
mover->Download(out_mem, outbuf->Size(), 0, ExecutionType::Sync);

// Dispose? We use RAII
```

## Currently tested

So far, we have tested CYNQ on:

1. Xilinx KV26-based with Ubuntu 2022.04
2. Xilinx Alveo U250 (it should be compatible with other similar Alveo cards) - Shell: xilinx_u250_gen3x16_xdma_4_1_202210_1

## Links & References:

* Docs: https://ecaslab.github.io/cynq
* Github: https://github.com/ECASLab/cynq

Cite Us:

```
@misc{cynq,
  author = {{León-Vega, Luis G.
              AND Ávila-Torres, Diego
              AND Castro-Godínez, Jorge
            }},
  title = {{CYNQ (v0.2)}},
  year  = {2024},
  url   = {https://github.com/ECASLab/cynq},
} 
```
