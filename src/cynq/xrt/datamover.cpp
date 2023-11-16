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

Status XRTDataMover::Upload(const std::shared_ptr<IMemory> /*mem*/,
                            const size_t /*size*/,
                            const ExecutionType /*exetype*/) {
  return Status{};
}

Status XRTDataMover::Download(const std::shared_ptr<IMemory> /*mem*/,
                              const size_t /*size*/,
                              const ExecutionType /*exetype*/) {
  return Status{};
}

Status XRTDataMover::Sync() { return Status{}; }

DeviceStatus XRTDataMover::GetStatus() { return DeviceStatus::Idle; }

XRTDataMover::~XRTDataMover() {
  /* The assumption is that at this point, it is ok */
  auto params =
      dynamic_cast<XRTDataMoverParameters *>(data_mover_params_.get());
  PYNQ_closeDMA(&params->dma_);
}
}  // namespace cynq
