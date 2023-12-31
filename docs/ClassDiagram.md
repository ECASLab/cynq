# Class Diagram

@startuml
interface IHardware {
  +{abstract} Reset() -> Status
  +{abstract} GetDataMover(address) -> IDataMover *
  +{abstract} GetAccelerator(address) -> IAccelerator *
  +{static} Create(hw: HardwareArchitecture, bitstream: string, xclbin: string) -> IHardware*

}

interface IMemory {
  {abstract} #GetHostAddress() -> uint8_t *
  {abstract} #GetDeviceAddress() -> uint8_t *
  +HostAddress<T>() -> T *
  +DeviceAddress<T>() -> T *
  {abstract} Sync(type: SyncType) -> Status
  {abstract} Size() -> size_t
  +{static} Create(impl: IMemoryType, size, hostptr, devptr) -> IMemory*
}

enum IMemoryType {
  XRT
  CMA
  ALIGNED
}

IMemoryType ..o IMemory

interface IAccelerator {
  {abstract} Start(mode: StartMode) -> Status
  {abstract} Stop() -> Status
  {abstract} #WriteRegister(address, data: uint8_t*, size: size_t) -> Status
  {abstract} #ReadRegister(address, data: uint8_t*, size: size_t) -> Status
  +Write<T>(address, data: T*, elems: size_t) -> Status
  +Read<T>(address, data: T*, elems: size_t) -> Status
  {abstract} GetStatus() -> DeviceStatus
  +{static} Create(impl: IAcceleratorType, addr: uint64) -> IAccelerator*
}

enum IAcceleratorType {
  XRT
  MMIO
  CHAR
}
IAcceleratorType ..o IAccelerator

interface IDataMover {
  {abstract} GetBuffer(size: size_t, type: MemoryType) -> IMemory *
  {abstract} Upload(mem: IMemory, size: size_t, exetype: ExecutionType) -> Status
  {abstract} Download(mem: IMemory, size: size_t, exetype: ExecutionType) -> Status
  {abstract} Sync() -> Status
  {abstract} GetStatus() -> DeviceStatus
  +{static} Create(impl: IDataMoverType, addr: uint64) -> IDataMover*
}

enum IDataMoverType {
  XRT
  DMA
  XDMA
}
IDataMoverType ..o IDataMover

enum HardwareArchitecture {
  UltraScale
  Zynq
  XDMA
  Alveo
}

HardwareArchitecture ..o IHardware

enum SyncType {
  HostToDevice,
  DeviceToHost,
}

enum StartMode {
  Once,
  Continuous
}

enum MemoryType {
  Dual,
  Cacheable,
  Host,
  Device
}

enum DeviceStatus {
  Unknown,
  Done,
  Idle,
  Running,
  Error
}

enum ExecutionType {
  Sync,
  Async
}


class UltraScale {
  +Reset() -> Status
  +GetDataMover(address) -> XRTDataMover *
  +GetAccelerator(address) -> AmdAccelerator *
  +UltraScale(hw, bitsteam, xclbin)
}

class XRTMemory {
  #GetHostAddress() -> uint8_t *
  #GetDeviceAddress() -> uint8_t *
  Sync(type: SyncType) -> Status
  Size() -> size_t
  +XRTMemory(hostptr, devptr)
}

class XRTAccelerator {
  Start(mode: StartMode) -> Status
  Stop() -> Status
  GetStatus() -> DeviceStatus
  #WriteRegister(address, data: uint8_t*, size: size_t) -> Status
  #ReadRegister(address, data: uint8_t*, size: size_t) -> Status
  +AmdAccelerator(addr: uint64)
}

class XRTDataMover {
  GetBuffer(size: size_t, type: MemoryType) -> XRTMemory *
  Upload(mem: IMemory, size: size_t, exetype: ExecutionType) -> Status
  Download(mem: IMemory, size: size_t, exetype: ExecutionType) -> Status
  Sync() -> Status
  GetStatus() -> DeviceStatus
  XRTDataMover(addr)
}

UltraScale ..> IHardware
XRTMemory ..> IMemory
XRTAccelerator ..> IAccelerator
XRTDataMover ..> IDataMover
@enduml
