# Foundations

## Current State

To develop on Xilinx FPGAs, there are a couple of workflows:

1. Legacy HLS -> Vivado workflow
2. Vitis Kernel workflow (newest)

The first workflow is general and works in most use cases, independently of the FPGA is intended for Cloud, Edge or instrumental use (like Spartan and Artix). The key artifact is a bitstream (.bit) used for configuring the FPGA, requiring only Vitis/Vivado HLS and Vivado Design Suite.

The second workflow is most specific for acceleration. It is used in modern MPSoCs and cloud-grade FPGAs. The key artifact is a binary (.xclbin) that is produced by a long chain of: Vitis HLS, Vivado Design Suite, Petalinux and Vitis. As you may notice, it's more tedious and requires more tooling, consuming more storage in the development machine.

PYNQ, particularly, has been characterised by its simplicity, only requiring the bitstream to work. It has also proved to be effective in a wide variety of boards, namely ZYNQ MPSoC and Alveo cards. In contrast, the new workflow requires more expertise in the tooling but produces better applications in terms of performance. However, the former one sacrifices a lot of the simplicity.

## Our Proposal

Looking at that, we want to propose a new library based on PYNQ to allow users to code efficient C++ application while keeping the simplicity of PYNQ, allowing even more features like:

* Support for fixed-point from `ap_fixed`
* Support for AXI-Stream without AXI-MM wrappers
* Workflow similar to the PYNQ

Within our plans, we want to:

* Integrate [AxC Executer](https://gitlab.com/ecas-lab-tec/approximate-flexible-acceleration-ml/axc-executer) for simulation.
* Integrate support for non-supported platforms on XRT or PYNQ, like the PicoEVB.
* Complete the support for Alveo, ZYNQ 7000 and Versal.
* Complete the support for XRT interoperability.
* Provide a library with math kernels.

This first release is a huge advance towards simplicity and we expect that many users look at CYNQ as the chance that they were waiting for to get started in the FPGA world, as seen in PYNQ.

## How does CYNQ work?

An application is mounted on top of an abstract interface to make the API feel agnostic. In this case, CYNQ is composed of five major components:

* Hardware class
* Accelerator class
* Data Mover class
* Memory class
* Execution Graph class

Depending on the hardware, these classes are implemented in different manners by using class extension. Thus, users won't feel any change when migrating their applications from one hardware to another. We can link these classes with the following equivalences:

| CYNQ Class            | PYNQ Component           |
|-----------------------|--------------------------|
| IHardware             | Overlay                  |
| IAccelerator          | Default IP               |
| IDataMover            | DMA IP                   |
| IMemory               | Buffer                   |
| IExecutionGraph       | N.A                      |

As it is possible to see, there is an equivalence at the functional level.

Going deeper, how CYNQ is currently mounted for the Xilinx Kria, it uses XRT for the buffers, MMIO for accelerators and data mover and FPGA manager for the hardware configuration.

See more in [Class Diagram](ClassDiagram.md)
