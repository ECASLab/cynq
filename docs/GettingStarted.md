# Getting Started

CYNQ is as straight-forward as PYNQ. The philosophy behind CYNQ is to be as simple as PYNQ.

1) The first step to integrate CYNQ is to include the header:

```c++
#include <cynq/cynq.hpp>
```

2) Create a IHardware object that will generate the proper drivers for handling the accelerator, data mover and memory. It uses the `IHardware::Create(impl, bitstream, xclbin)` factory. For instance:

```c++
auto impl = cynq::HardwareArchitecture::UltraScale;
auto platform = 
    cynq::IHardware::Create(impl, bitstream, xclbin);
```

where:

* `impl` is the HardwareArchitecture. At the moment, only `UltraScale` is supported for UltraScale MPSoC.
* `bitstream` is the path to the .bit file (bitstream)
* `xclbin` is the path to the .xclbin file. You can use the one in `third-party/resources/default.xclbin`.

3) Create the DMA instances to move the data. This is intended for designs that uses AXI4-Stream.

```c++
constexpr int kDmaAddress = 0xa0010000;
auto dma = platform->GetDataMover(kDmaAddress);
```

where `kDmaAddress` is the address of the DMA instance we want to control. This is given by the design.

4) Create the IP core instances to interact with them.

```c++
constexpr int kAccelAddress = 0xa0000000;
auto accel = platform->GetAccelerator(kAccelAddress);
```

where `kAccelAddress` is the AXI4-Lite control port of the IP core. As a requirement:

> The IP Cores must have an AXI4-Lite port as a control port

5) Get buffers to exchange data. These buffers are usually dual memory: they are mapped into host and device regions (physically contiguous).

```c++
std::size_t input_size = 64; // bytes
std::size_t output_size = 64; // bytes
auto type = MemoryType::Dual; // dual memory

auto in_mem = dma->GetBuffer(input_size, type);
auto out_mem = dma->GetBuffer(output_size, type);
```

where the `GetBuffer()` method includes: `size` in bytes and `type` of the memory type:

* `cynq::MemoryType::Dual`: allocates two memory regions, one accessible from host and another from device.
* `cynq::MemoryType::Cacheable`: allocates a memory region which is cacheable.
* `cynq::MemoryType::Host`: allocates a host-only memory.
* `cynq::MemoryType::Device`: allocates a device-only memory.

6) To access the data from the memory buffers, you can use the `HostAddress` method.

```c++
using DataType = uint64_t;
DataType* A = in_mem->HostAddress<DataType>().get();

A[5] = 1;
```

The `HostAddress<T>()` maps the memory into a pointer that is accessible to the host. `T` can be any type that can be reinterpretedly casted.

7) Write/Read the IP Core / Accelerator registers. You can use the `Write()` and `Read()` methods.

```c++
uint16_t input_a_cols;

// Read a single element
accel->Read(0x20, &input_a_cols, 1);

// Write a single element
input_a_cols = 64;
accel->Write(0x28, &input_a_cols, 1);
```

Both `Read(addr, data*, elems)` and `Write(addr, data*, elems)` have the same arguments:

* `addr`: offset address of the register
* `data*`: data pointer to be read/written
* `elems`: number of elements of type `data` to read/write

8) Start/Stop the accelerator by writing the control register

```c++
accel->Start(StartMode::Continuous);

accel->Stop();
```

To start the accelerator, you can use the `Start()` method, which receives either of the following values:

* `cynq::StartMode::Once`: turns on the accelerator 
* `cynq::StartMode::Continuous`: turns on the accelerator in autorestart mode.

9) Transferring information requires the synchronisation of the memory buffers and the interaction with the data mover / DMA.

To synchronise the buffers, it is possible to use `Sync()` method: 

```c++
in_mem->Sync(SyncType::HostToDevice);
out_mem->Sync(SyncType::DeviceToHost);
```

that takes one of the following values:

* `cynq::SyncType::DeviceToHost`: device to host synchronisation
* `cynq::SyncType::HostToDevice`: host to device synchronisation

Once the memory is synchronised, the data mover is used to upload the data to the AXI4-Stream or download from it.

```c++
// Upload: requires the buffer to be sync in HostToDevice
dma->Upload(in_mem, in_mem->Size(), 0, ExecutionType::Sync);
// Download: after its completion, the buffer must be sync DeviceToHost
dma->Download(out_mem, out_mem->Size(), 0, ExecutionType::Sync);
```

Both methods takes: `(memory, size, offset, execution_type)`, where `size` is the amount of data to transfer in bytes, `offset` moves the starting point of the data and `execution_type` is the type of execution:

* `cynq::ExecutionType::Sync`: synchronous mode
* `cynq::ExecutionType::Async`: asynchronous mode

If the `execution_type` is `cynq::ExecutionType::Async`, the synchronisation can be done with `Sync(sync_type)`:

* `cynq::SyncType::DeviceToHost`: device to host synchronisation
* `cynq::SyncType::HostToDevice`: host to device synchronisation

10) The disposal is done automatically thanks to C++ RAII.
