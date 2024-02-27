/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *
 */

#include <xrt/xrt/xrt_bo.h>

#include <cynq/alveo/hardware.hpp>
#include <cynq/datamover.hpp>
#include <cynq/enums.hpp>
#include <cynq/hardware.hpp>
#include <cynq/memory.hpp>
#include <cynq/status.hpp>
#include <cynq/xrt/datamover.hpp>
#include <memory>

namespace cynq {
/**
 * @brief Define the specialisation of the data mover with the XRT engine
 * and XRT
 */
struct XRTDataMoverParameters : public DataMoverParameters {
  /** Virtual destructor required for the inheritance */
  virtual ~XRTDataMoverParameters() = default;
};

XRTDataMover::XRTDataMover(const uint64_t /* addr */,
                           std::shared_ptr<HardwareParameters> hwparams)
    : data_mover_params_{std::make_unique<XRTDataMoverParameters>()} {
  /* The assumption is that at this point, it is ok */
  auto params =
      dynamic_cast<XRTDataMoverParameters *>(data_mover_params_.get());
  params->hw_params_ = hwparams;
}

std::shared_ptr<IMemory> XRTDataMover::GetBuffer(const size_t size,
                                                 const MemoryType type,
                                                 const uint memory_bank) {
  const xrt::memory_group group = (xrt::memory_group)(memory_bank);

  /* The assumption is that at this point, it is ok */
  auto hw_params_ =
      dynamic_cast<AlveoParameters *>(data_mover_params_->hw_params_.get());
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

XRTDataMover::~XRTDataMover() {}

/* TODO: All implementations below can be implemented cleverly. However, it
   was out from the scope of this release. In next releases, we can have a
   concept similar to work streams or queues */

Status XRTDataMover::Upload(const std::shared_ptr<IMemory> mem,
                            const size_t size, const size_t offset,
                            const ExecutionType /* exetype */) {
  /* Get device pointer */
  if (!mem) {
    return Status{Status::INVALID_PARAMETER, "Memory pointer is null"};
  }

  auto xrtmem = dynamic_cast<XRTMemory *>(mem.get());

  /* Verify the sizes and offsets */
  if ((size + offset) > mem->Size()) {
    return Status{Status::INVALID_PARAMETER,
                  "The offset and size exceeds the memory size"};
  }

  auto meta = (XRTDataMoverMeta *)(xrtmem->mover_ptr_);  // NOLINT
  meta->bo_->sync(XCL_BO_SYNC_BO_TO_DEVICE, size, offset);

  return Status{};
}

Status XRTDataMover::Download(const std::shared_ptr<IMemory> mem,
                              const size_t size, const size_t offset,
                              const ExecutionType /* exetype */) {
  /* Get device pointer */
  if (!mem) {
    return Status{Status::INVALID_PARAMETER, "Memory pointer is null"};
  }

  auto xrtmem = dynamic_cast<XRTMemory *>(mem.get());

  /* Verify the sizes and offsets */
  if ((size + offset) > mem->Size()) {
    return Status{Status::INVALID_PARAMETER,
                  "The offset and size exceeds the memory size"};
  }

  auto meta = (XRTDataMoverMeta *)(xrtmem->mover_ptr_);  // NOLINT
  meta->bo_->sync(XCL_BO_SYNC_BO_FROM_DEVICE, size, offset);

  return Status{};
}

Status XRTDataMover::Sync(const SyncType /* type */) {
  /* TODO: not implemented. We must find a way to synchronise in another
     fashion. An idea is to launch std::async jobs and wait until its
     synchronisation */
  return Status{};
}
}  // namespace cynq
