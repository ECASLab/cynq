/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */

#include <xrt/xrt_bo.h>

#include <cynq/datamover.hpp>
#include <cynq/dma/datamover.hpp>
#include <cynq/enums.hpp>
#include <cynq/hardware.hpp>
#include <cynq/memory.hpp>
#include <cynq/status.hpp>
#include <cynq/ultrascale/hardware.hpp>
#include <memory>

extern "C" {
#include <pynq_api.h> /* FIXME: to be removed in future releases */
}

namespace cynq {
/**
 * @brief Define the specialisation of the data mover with the DMA engine
 * and XRT
 */
struct DMADataMoverParameters : public DataMoverParameters {
  /** DMA accessor */
  PYNQ_AXI_DMA dma_;
  /** DMA address */
  uint64_t addr_;
  /** Virtual destructor required for the inheritance */
  virtual ~DMADataMoverParameters() = default;
};

DMADataMover::DMADataMover(const uint64_t addr,
                           std::shared_ptr<HardwareParameters> hwparams)
    : data_mover_params_{std::make_unique<DMADataMoverParameters>()} {
  /* The assumption is that at this point, it is ok */
  auto params =
      dynamic_cast<DMADataMoverParameters *>(data_mover_params_.get());

  params->addr_ = addr;
  params->hw_params_ = hwparams;

  /* Create the DMA accessor */
  if (static_cast<uint64_t>(0ul) != addr) {
    PYNQ_openDMA(&params->dma_, addr);
  }
}

std::shared_ptr<IMemory> DMADataMover::GetBuffer(const size_t size, const int,
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
  DMADataMoverMeta *meta = new DMADataMoverMeta;
  meta->bo_ = buffer_object;
  meta->type_ = type;

  return IMemory::Create(IMemory::XRT, size, nullptr, nullptr,
                         reinterpret_cast<void *>(meta));
}

DeviceStatus DMADataMover::GetStatus() { return DeviceStatus::Idle; }

DMADataMover::~DMADataMover() {
  /* The assumption is that at this point, it is ok */
  auto params =
      dynamic_cast<DMADataMoverParameters *>(data_mover_params_.get());
  if (static_cast<uint64_t>(0ul) != params->addr_) {
    PYNQ_closeDMA(&params->dma_);
  }
}

/* TODO: All implementations below can be implemented cleverly. However, it
   was out from the scope of this release. In next releases, we can have a
   concept similar to work streams or queues */

Status DMADataMover::Upload(const std::shared_ptr<IMemory> mem,
                            const size_t size, const size_t offset,
                            const ExecutionType exetype) {
  int ret = PYNQ_SUCCESS;
  auto params =
      dynamic_cast<DMADataMoverParameters *>(data_mover_params_.get());

  if (!mem) {
    return Status{Status::INVALID_PARAMETER, "Memory pointer is null"};
  }

  /* Verify the sizes and offsets */
  if ((size + offset) > mem->Size()) {
    return Status{Status::INVALID_PARAMETER,
                  "The offset and size exceeds the memory size"};
  }

  /* Get the actual memory and the meta */
  auto xrtmem = dynamic_cast<XRTMemory *>(mem.get());
  auto meta = (DMADataMoverMeta *)(xrtmem->mover_ptr_);  // NOLINT
  if (meta) {
    meta->bo_->sync(XCL_BO_SYNC_BO_TO_DEVICE, size, offset);
  }

  /* Issue transaction */
  if (static_cast<uint64_t>(0ul) != params->addr_) {
    /* Get device pointer */
    std::shared_ptr<uint8_t> ptr = mem->DeviceAddress<uint8_t>();
    if (!ptr) {
      return Status{Status::INVALID_PARAMETER, "Device pointer is null"};
    }

    PYNQ_SHARED_MEMORY pmem;
    pmem.physical_address = (uint64_t)(ptr.get());  // NOLINT
    pmem.pointer = nullptr;

    ret = PYNQ_issueDMATransfer(&params->dma_, &pmem, offset, size,
                                AXI_DMA_WRITE);

    /* Check transaction */
    if (PYNQ_SUCCESS != ret) {
      return Status{Status::REGISTER_IO_ERROR, "Cannot issue the transfer"};
    }
  }

  /* Synchronise if needed */
  if (ExecutionType::Async == exetype) {
    return Status{};
  } else {
    return this->Sync(SyncType::HostToDevice);
  }
}

Status DMADataMover::Download(const std::shared_ptr<IMemory> mem,
                              const size_t size, const size_t offset,
                              const ExecutionType exetype) {
  int ret = PYNQ_SUCCESS;
  auto params =
      dynamic_cast<DMADataMoverParameters *>(data_mover_params_.get());

  if (!mem) {
    return Status{Status::INVALID_PARAMETER, "Memory pointer is null"};
  }

  /* Verify the sizes and offsets */
  if ((size + offset) > mem->Size()) {
    return Status{Status::INVALID_PARAMETER,
                  "The offset and size exceeds the memory size"};
  }

  /* Get the actual memory and the meta */
  auto xrtmem = dynamic_cast<XRTMemory *>(mem.get());
  auto meta = (DMADataMoverMeta *)(xrtmem->mover_ptr_);  // NOLINT
  if (meta) {
    meta->bo_->sync(XCL_BO_SYNC_BO_FROM_DEVICE, size, offset);
  }

  /* Issue transaction */
  if (static_cast<uint64_t>(0ul) != params->addr_) {
    std::shared_ptr<uint8_t> ptr = mem->DeviceAddress<uint8_t>();

    /* Get device pointer */
    if (!ptr) {
      return Status{Status::INVALID_PARAMETER, "Device pointer is null"};
    }

    PYNQ_SHARED_MEMORY pmem;
    pmem.physical_address = (uint64_t)(ptr.get());  // NOLINT
    pmem.pointer = nullptr;
    ret =
        PYNQ_issueDMATransfer(&params->dma_, &pmem, offset, size, AXI_DMA_READ);

    /* Check transaction */
    if (PYNQ_SUCCESS != ret) {
      return Status{Status::REGISTER_IO_ERROR, "Cannot issue the transfer"};
    }
  }

  /* Synchronise if needed */
  if (ExecutionType::Async == exetype) {
    return Status{};
  } else {
    return this->Sync(SyncType::DeviceToHost);
  }
}

Status DMADataMover::Sync(const SyncType type) {
  auto params =
      dynamic_cast<DMADataMoverParameters *>(data_mover_params_.get());

  if (static_cast<uint64_t>(0ul) == params->addr_) {
    return Status{};
  }

  int ret = PYNQ_SUCCESS;

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
