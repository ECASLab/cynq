# Getting Started

CYNQ is as straight-forward as PYNQ. The philosophy behind CYNQ is to be as simple as PYNQ.

## Xilinx K26 SoM

1) The first step to integrate CYNQ is to include the header:

```c++
#include <cynq/cynq.hpp>
```

2) Create an IHardware object to generate the proper drivers for handling the accelerator, data mover and memory. It uses the `IHardware::Create(impl, bitstream, xclbin)` factory. For instance:

```c++
auto impl = cynq::HardwareArchitecture::UltraScale;
auto platform = 
    cynq::IHardware::Create(impl, bitstream, xclbin);
```

where:

* `impl` is the HardwareArchitecture. At the moment, only `UltraScale` is supported for UltraScale MPSoC.
* `bitstream` is the path to the .bit file (bitstream)
* `xclbin` is the path to the .xclbin file. You can use the one in `third-party/resources/default.xclbin`.

3) Create the DMA instances to move the data. This is intended for designs that use AXI4-Stream.

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

5) Get buffers to exchange data. These buffers are usually dual memory mapped into host and device regions (physically contiguous).

```c++
std::size_t input_size = 64; // bytes
std::size_t output_size = 64; // bytes
auto type = MemoryType::Dual; // dual memory

auto in_mem = dma->GetBuffer(input_size, type);
auto out_mem = dma->GetBuffer(output_size, type);
```

where the `GetBuffer()` method includes: `size` in bytes and `type` of the memory type:

* `cynq::MemoryType::Dual`: allocates two memory regions, one accessible from host and another from device.
* `cynq::MemoryType::Cacheable`: allocates a cacheable memory region.
* `cynq::MemoryType::Host`: allocates a host-only memory.
* `cynq::MemoryType::Device`: allocates a device-only memory.

6) To access the data from the memory buffers, you can use the `HostAddress` method.

```c++
using DataType = uint64_t;
DataType* A = in_mem->HostAddress<DataType>().get();

A[5] = 1;
```

The `HostAddress<T>()` maps the memory into a pointer that is accessible to the host. `T` can be any type that can be reinterpretedly casted.

7) Write/Read the IP Core / Accelerator registers. You can use the `Write()` and `Read()` methods for AXI4-Lite interfaces.

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

Moreover, if you need to attach a memory block to an AXI4 Memory Mapped interface whose address is defined in ab AXI4-Lite interface, you can use the Attach(addr, buffer).

```c++
uint32_t addr = 0x40
accel->Attach(addr, mem_bo);
```

`Attach(addr, data)` arguments:

* `addr`: memory address offset in the AXI4-Lite control register bank.
* `mem`: memory buffer to attach (`std::shared_ptr<IMemory>`)

**New!**: You can use attach also for parameters instead of using `Write()`/`Read()`:

For read-only (from the accelerator BAR):

```c++
accel->Attach(0x20, &input_a_cols, RegisterAccess::RO, 1);
```

or for write-only (also from the accelerator BAR):

```c++
accel->Attach(0x28, &input_a_cols, RegisterAccess::RO, 1);
```

The `Attach(addr, data, type, elems)` arguments are:

* `addr`: memory address offset in the AXI4-Lite control register bank.
* `data`: pointer to host memory where the register's values are held.
* `type`: type of transfer (see cynq::RegisterAccess)
* `elems`: number of elements of type of data (the memory must be 32-bit aligned).

Important:

The registers are written only when the `IAccelerator::Start` is invoked. This does not apply to `StartMode::Continuous` since the writes only happen every invocation of the `IAccelerator::Start` method. The registers are read back when invoking  IAccelerator::Stop` or `IAccelerator::Sync`.

8) Start/Stop the accelerator by writing the control register

```c++
accel->Start(StartMode::Continuous);

accel->Stop();
```

To start the accelerator, you can use the `Start()` method, which receives either of the following values:

* `cynq::StartMode::Once`: turns on the accelerator 
* `cynq::StartMode::Continuous`: turns on the accelerator in auto-restart mode.

9) Transferring information requires the synchronisation of the memory buffers and the interaction with the data mover / DMA.

The data mover is used to upload the data to the AXI4-Stream or download from it.

```c++
// Upload: requires the buffer to be sync in HostToDevice
dma->Upload(in_mem, in_mem->Size(), 0, ExecutionType::Sync);
// Download: after its completion, the buffer must be sync DeviceToHost
dma->Download(out_mem, out_mem->Size(), 0, ExecutionType::Sync);
```

Both methods take: `(memory, size, offset, execution_type)`, where `size` is the amount of data to transfer in bytes, `offset` moves the starting point of the data and `execution_type` is the type of execution:

* `cynq::ExecutionType::Sync`: synchronous mode
* `cynq::ExecutionType::Async`: asynchronous mode

10) The disposal is done automatically thanks to C++ RAII.

### Clocking

Since we do not read the .hwh file, we are not aware about the clocks. We require clocks to properly run the accelerators at the right speed. It is possible to fix the PLL divisors through the IHardware::SetClocks method. They are used as follows:

```c++
auto clocks = platform->GetClocks();
clocks[0] = 250.f;
platform->SetClocks(clocks);
```

where `platform` is an `IHardware` instance and `250.f` means `250 MHz`.

## Alveo Cards or XRT-based platforms with Vitis workflow

1) The first step to integrate CYNQ is to include the header:

```c++
#include <cynq/cynq.hpp>
```

2) Create an IHardware object to create the proper drivers for handling the accelerator, data mover and memory. It uses the `IHardware::Create(impl, bitstream, xclbin)` factory. For instance:

```c++
auto impl = cynq::HardwareArchitecture::Alveo;
auto platform = 
    cynq::IHardware::Create(impl, "", xclbin);
```

where:

* `impl` is the HardwareArchitecture. At the moment, only `UltraScale` is supported for UltraScale MPSoC.
* `xclbin` is the path to the .xclbin file. You can use the one in `third-party/resources/alveo-xclbin/vadd.xclbin`.

3) Create the DMA instances to move the data. This is intended for designs that use AXI4-Stream.

```c++
auto dma = platform->GetDataMover(0);
```

where `0` is a dummy value, and it is currently unused in this implementation.

4) Create the IP core instances to interact with them.

```c++
auto accel = platform->GetAccelerator("vadd");
```

where `"vadd"` is the kernel compiled with `v++`. It is based on the following kernel: [vadd.cpp](https://github.com/Xilinx/Vitis_Accel_Examples/blob/2022.1/host_xrt/hello_world_xrt/src/vadd.cpp]

5) Get buffers to exchange data. These buffers are usually dual memory mapped into host and device regions (physically contiguous).

```c++
std::size_t vec_size = sizeof(int) * kDataSize;
auto type = MemoryType::Dual;
auto bo_0 = mover->GetBuffer(vec_size, accel->GetMemoryBank(0), type);;
```

where the `GetBuffer()` method includes: `size` in bytes and `type` of the memory type:

* `cynq::MemoryType::Dual`: allocates two memory regions, one accessible from host and another from device.
* `cynq::MemoryType::Cacheable`: allocates a cacheable memory region.
* `cynq::MemoryType::Host`: allocates a host-only memory.
* `cynq::MemoryType::Device`: allocates a device-only memory.

6) To access the data from the memory buffers, you can use the `HostAddress` method.

```c++
auto bo_0_map = bo_0->HostAddress<int>().get();
bo_0_map[5] = 1;
```

The `HostAddress<T>()` maps the memory into a pointer that is accessible to the host. `T` can be any type that can be reinterpretedly casted.

7) Write/Read the IP Core / Accelerator registers. You can use the `Attach()` method.

If you must attach a memory to an AXI4 Memory Mapped interface, you can instantiate the IMemory pointer through Attach(index, buffer).

```c++
accel->Attach(0, bo_0);
```

`Attach(index, data)` arguments:

* `index`: argument position
* `mem`: memory buffer to attach

If you require to attach an argument that is either a scalar or an array in AXI4-Lite interfaces, you can also use Attach(index, data*, n), where:

* index: position of the argument in the kernel
* data: scalar or array pointer
* n: number of elements

For example:

```c++
uint datasize = 4096;
accel->Attach(3, &datasize);
```

8) Upload data

The data upload is done through the data mover:

```c++
mover->Upload(bo_0, bo_0->Size(), 0, ExecutionType::Async);
```

the `Upload(mem, size, offset, execution_type)` function is used to upload data from host to device where the arguments are:

* mem: memory buffer of type `std::shared_ptr<IMemory>`
* size: size in bytes
* offset: starting point of the buffer
* execution_type: synchronisation type
  * `cynq::ExecutionType::Sync`: synchronous mode
  * `cynq::ExecutionType::Async`: asynchronous mode

9) Start/Stop the accelerator by writing the control register

```c++
accel->Start(StartMode::Once);
accel->Sync();
```

To start the accelerator, you can use the `Start()` method, which receives either of the following values:

* `cynq::StartMode::Once`: turns on the accelerator 
* `cynq::StartMode::Continuous`: turns on the accelerator in auto-restart mode (not supported in Alveo).

10) Download data

The data download is done through the data mover:

```c++
mover->Download(bo_0, bo_0->Size(), 0, ExecutionType::Sync);
```

the `Download(mem, size, offset, execution_type)` function is used to download data from host to device where the arguments are:

* mem: memory buffer of type `std::shared_ptr<IMemory>`
* size: size in bytes
* offset: starting point of the buffer
* execution_type: synchronisation type
  * `cynq::ExecutionType::Sync`: synchronous mode
  * `cynq::ExecutionType::Async`: asynchronous mode

11) The disposal is done automatically, thanks to C++ RAII.

## Using Execution Graphs

From v0.3, CYNQ integrates execution graphs. Currently, they are based on execution queues as in CUDA (CUDA Stream). The idea is to add asynchronous non-blocking execution to CYNQ to offer more flexibility. Here there are some tips:

* Create the execution graph from the platform:

```c++
auto stream = platform->GetExecutionStream("mystream");
```

* Use a stream per accelerator.

Thus, you can use the accelerators in parallel without worrying about sequential execution of each accelerator. This can lower your execution time.

* Avoid having a single accelerator in different streams.

Calling an accelerator from different streams may cause race conditions and unexpected behaviours. We are currently working on covering this scenarios so that there are no worries in this matter.

* Avoid oversynchronisation.

You can call asynchronous code within the stream. However, take into account that you may need to synchronise at a certain point. So, IAccelerator::Sync, IDataMover::Sync and IMemory::Sync can be also added to the streams.

### Converting serial code to stream-based parallel code

Basically, you can code sequentially. If you want to move it to a stream, create a stream and add it to the first argument.

For example:

_Sequential code:_

```c++
matmul->Write(XMATMUL_CONTROL_ADDR_A_ROWS_DATA, &a_rows, 1);
matmul->Write(XMATMUL_CONTROL_ADDR_B_COLS_DATA, &b_cols, 1);
matmul->Write(XMATMUL_CONTROL_ADDR_C_COLS_DATA, &c_cols, 1);
buf_mem_mm_a->Sync(SyncType::HostToDevice);
buf_mem_mm_b->Sync(SyncType::HostToDevice);
matmul->Start(StartMode::Once);
matmul->Sync();
buf_mem_mm_c->Sync(SyncType::DeviceToHost);
```

_Stream code:_

```c++
matmul->Write(stream, XMATMUL_CONTROL_ADDR_A_ROWS_DATA, &a_rows, 1);
matmul->Write(stream, XMATMUL_CONTROL_ADDR_B_COLS_DATA, &b_cols, 1);
matmul->Write(stream, XMATMUL_CONTROL_ADDR_C_COLS_DATA, &c_cols, 1);
buf_mem_mm_a->Sync(stream, SyncType::HostToDevice);
buf_mem_mm_b->Sync(stream, SyncType::HostToDevice);
matmul->Start(stream, StartMode::Once);
matmul->Sync(stream);
buf_mem_mm_c->Sync(stream, SyncType::DeviceToHost);
```

Examples: zynq-mpsoc/ad08-sequential.cpp, zynq-mpsoc/ad08-streams.cpp

### Adding non-CYNQ function to the stream

You can also add code which is not part of CYNQ. Here is an example:

```c++
volatile std::atomic_int num{0};

// Here is the function to add
cynq::Status dummy_function() {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "num: " << num.load() << std::endl;
  num++;
  return cynq::Status{};
}

// Add your function to the stream
stream->Add(dummy_function);
```

You can also add a function multiple times.

For an example, you can look at structures/execution-stream.cpp.
