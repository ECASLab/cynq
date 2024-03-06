/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include "cynq/memory.hpp"

#include <xrt/xrt_bo.h>

#include <memory>

#include "cynq/dma/datamover.hpp"
#include "cynq/enums.hpp"
#include "cynq/status.hpp"
#include "cynq/xrt/memory.hpp"

namespace cynq {
XRTMemory::XRTMemory(const std::size_t size, uint8_t *hostptr, uint8_t *devptr,
                     void *moverptr)
    : size_{size}, host_ptr_{hostptr}, dev_ptr_{devptr}, mover_ptr_{moverptr} {}

Status XRTMemory::Sync(const SyncType type) {
  if (!mover_ptr_) {
    return Status{Status::NOT_IMPLEMENTED, "Don't know how to synchronise"};
  }

  /* Determine the direction */
  auto meta = reinterpret_cast<DMADataMoverMeta *>(mover_ptr_);
  xclBOSyncDirection dir = type == SyncType::HostToDevice
                               ? XCL_BO_SYNC_BO_TO_DEVICE
                               : XCL_BO_SYNC_BO_FROM_DEVICE;

  if (!meta->bo_) {
    return Status{Status::MEMBER_ABSENT, "Cannot find a valid BO"};
  }

  /* Synchronise */
  meta->bo_->sync(dir);

  return Status{};
}

size_t XRTMemory::Size() { return size_; }

std::shared_ptr<uint8_t> XRTMemory::GetHostAddress() {
  /* Relevant: the returning shared pointer has no deleter since it is not
   * transfer full */
  if (!mover_ptr_) {
    return std::shared_ptr<uint8_t>(host_ptr_, [](uint8_t *) {});
  } else {
    auto meta = reinterpret_cast<DMADataMoverMeta *>(mover_ptr_);

    if (meta->type_ == MemoryType::Device) {
      return nullptr;
    }

    if (!meta->bo_) {
      return nullptr;
    }

    return std::shared_ptr<uint8_t>(meta->bo_->map<uint8_t *>(),
                                    [](uint8_t *) {});
  }
}

std::shared_ptr<uint8_t> XRTMemory::GetDeviceAddress() {
  /* Relevant: the returning shared pointer has no deleter since it is not
   * transfer full */
  if (!mover_ptr_) {
    return std::shared_ptr<uint8_t>(dev_ptr_, [](uint8_t *) {});
  } else {
    auto meta = reinterpret_cast<DMADataMoverMeta *>(mover_ptr_);

    if (meta->type_ == MemoryType::Host) {
      return nullptr;
    }

    if (!meta->bo_) {
      return nullptr;
    }

    /* Construct the pointer */
    uint64_t addr = meta->bo_->address();
    uint8_t *ptr = reinterpret_cast<uint8_t *>(addr);
    return std::shared_ptr<uint8_t>(ptr, [](uint8_t *) {});
  }
}

XRTMemory::~XRTMemory() {
  if (mover_ptr_) {
    auto meta = reinterpret_cast<DMADataMoverMeta *>(mover_ptr_);
    delete meta;
  }
}
}  // namespace cynq
