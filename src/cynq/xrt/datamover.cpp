/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */

#include <cynq/xrt/datamover.hpp>

#include <memory>

#include <xrt/xrt/xrt_bo.h>

#include <cynq/datamover.hpp>
#include <cynq/enums.hpp>
#include <cynq/hardware.hpp>
#include <cynq/memory.hpp>
#include <cynq/status.hpp>

#include <cynq/ultrascale/hardware.hpp>

extern "C" {
#include <pynq_api.h> /* FIXME: to be removed in future releases */
}

namespace cynq {
/**
 * @brief Define the specialisation of the data mover with the DMA engine
 * and XRT
 */
struct XRTDataMoverParameters : public DataMoverParameters {
  /** DMA accessor */
  PYNQ_AXI_DMA dma_;
  /** DMA address */
  uint64_t addr_;
  /** Virtual destructor required for the inheritance */
  virtual ~XRTDataMoverParameters() = default;
};

XRTDataMover::XRTDataMover(const uint64_t addr,
                           std::shared_ptr<HardwareParameters> hwparams)
    : data_mover_params_{std::make_unique<XRTDataMoverParameters>()} {
  /* The assumption is that at this point, it is ok */
  auto params =
      dynamic_cast<XRTDataMoverParameters *>(data_mover_params_.get());

  params->addr_ = addr;
  params->hw_params_ = hwparams;

  /* Create the DMA accessor */
  PYNQ_openDMA(&params->dma_, addr);
}

std::shared_ptr<IMemory> XRTDataMover::GetBuffer(const size_t size,
                                                 const MemoryType type) {
  const xrt::memory_group group = (xrt::memory_group)(0);

  /* The assumption is that at this point, it is ok */
  auto hw_params_ = dynamic_cast<UltraScaleParameters *>(
      data_mover_params_->hw_params_.get());
  if (!hw_params_) {
    throw std::runtime_error("Hardware params are incompatible");
  }

  xrt::bo::flags flags;
  switch (type) {
    case MemoryType::Cacheable:
      flags = xrt::bo::flags::cacheable;
      break;
    case MemoryType::Device:
      flags = xrt::bo::flags::device_only;
      break;
    case MemoryType::Host:
      flags = xrt::bo::flags::host_only;
      break;
    default:
      flags = xrt::bo::flags::normal;
      break;
  }

  /* Create the buffer object and encapsulate it into the meta. The meta is FULL
   * TRANSFER */
  auto buffer_object =
      std::make_shared<xrt::bo>(hw_params_->device_, size, flags, group);
  XRTDataMoverMeta *meta = new XRTDataMoverMeta;
  meta->bo_ = buffer_object;
  meta->type_ = type;

  return IMemory::Create(IMemory::XRT, size, nullptr, nullptr,
                         reinterpret_cast<void *>(meta));
}

DeviceStatus XRTDataMover::GetStatus() { return DeviceStatus::Idle; }

XRTDataMover::~XRTDataMover() {
  /* The assumption is that at this point, it is ok */
  auto params =
      dynamic_cast<XRTDataMoverParameters *>(data_mover_params_.get());
  PYNQ_closeDMA(&params->dma_);
}

/* TODO: All implementations below can be implemented cleverly. However, it
   was out from the scope of this release. In next releases, we can have a
   concept similar to work streams or queues */

Status XRTDataMover::Upload(const std::shared_ptr<IMemory> mem,
                            const size_t size, const size_t offset,
                            const ExecutionType exetype) {
  int ret = PYNQ_SUCCESS;
  auto params =
      dynamic_cast<XRTDataMoverParameters *>(data_mover_params_.get());

  /* Get device pointer */
  if (!mem) {
    return Status{Status::INVALID_PARAMETER, "Memory pointer is null"};
  }
  std::shared_ptr<uint8_t> ptr = mem->DeviceAddress<uint8_t>();
  if (!ptr) {
    return Status{Status::INVALID_PARAMETER, "Device pointer is null"};
  }

  /* Verify the sizes and offsets */
  if ((size + offset) > mem->Size()) {
    return Status{Status::INVALID_PARAMETER,
                  "The offset and size exceeds the memory size"};
  }

  /* Issue transaction */
  PYNQ_SHARED_MEMORY pmem;
  pmem.physical_address = (unsigned long)(ptr.get());  // NOLINT
  pmem.pointer = nullptr;

  ret =
      PYNQ_issueDMATransfer(&params->dma_, &pmem, offset, size, AXI_DMA_WRITE);

  /* Check transaction */
  if (PYNQ_SUCCESS != ret) {
    return Status{Status::REGISTER_IO_ERROR, "Cannot issue the transfer"};
  }

  /* Synchronise if needed */
  if (ExecutionType::Async == exetype) {
    return Status{};
  } else {
    return this->Sync(SyncType::HostToDevice);
  }
}

Status XRTDataMover::Download(const std::shared_ptr<IMemory> mem,
                              const size_t size, const size_t offset,
                              const ExecutionType exetype) {
  int ret = PYNQ_SUCCESS;
  auto params =
      dynamic_cast<XRTDataMoverParameters *>(data_mover_params_.get());

  /* Get device pointer */
  if (!mem) {
    return Status{Status::INVALID_PARAMETER, "Memory pointer is null"};
  }
  std::shared_ptr<uint8_t> ptr = mem->DeviceAddress<uint8_t>();
  if (!ptr) {
    return Status{Status::INVALID_PARAMETER, "Device pointer is null"};
  }

  /* Verify the sizes and offsets */
  if ((size + offset) > mem->Size()) {
    return Status{Status::INVALID_PARAMETER,
                  "The offset and size exceeds the memory size"};
  }

  /* Issue transaction */
  PYNQ_SHARED_MEMORY pmem;
  pmem.physical_address = (unsigned long)(ptr.get());  // NOLINT
  pmem.pointer = nullptr;
  ret = PYNQ_issueDMATransfer(&params->dma_, &pmem, offset, size, AXI_DMA_READ);

  /* Check transaction */
  if (PYNQ_SUCCESS != ret) {
    return Status{Status::REGISTER_IO_ERROR, "Cannot issue the transfer"};
  }

  /* Synchronise if needed */
  if (ExecutionType::Async == exetype) {
    return Status{};
  } else {
    return this->Sync(SyncType::DeviceToHost);
  }
}

Status XRTDataMover::Sync(const SyncType type) {
  int ret = PYNQ_SUCCESS;

  auto params =
      dynamic_cast<XRTDataMoverParameters *>(data_mover_params_.get());

  if (SyncType::HostToDevice == type) {
    ret = PYNQ_waitForDMAComplete(&params->dma_, AXI_DMA_WRITE);
  } else {
    ret = PYNQ_waitForDMAComplete(&params->dma_, AXI_DMA_READ);
  }

  if (PYNQ_SUCCESS != ret) {
    return Status{Status::REGISTER_IO_ERROR, "Cannot synchronise"};
  }

  return Status{};
}
}  // namespace cynq
