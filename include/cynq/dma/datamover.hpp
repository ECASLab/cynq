/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#pragma once
#include <xrt/xrt_bo.h>

#include <cynq/datamover.hpp>
#include <cynq/enums.hpp>
#include <cynq/hardware.hpp>
#include <cynq/status.hpp>
#include <cynq/xrt/memory.hpp>
#include <memory>

namespace cynq {
/**
 * @brief Metadata inserted into each XRTMemory instance
 */
struct DMADataMoverMeta {
  /** Buffer object */
  std::shared_ptr<xrt::bo> bo_;
  /** Memory type */
  MemoryType type_;
};

/**
 * @brief DMADataMover class
 * Provides the api from which to interact with the data buffers responsable for
 * memory operations making use of the Xilinx runtime (XRT) the types are the
 * following: BO (Buffer object) DMA (Direct memory allocation)
 *
 */
class DMADataMover : public IDataMover {
 public:
  /**
   * @brief Construct a new DMADataMover object
   *
   * This constructs a data mover that uses DMA to execute the transfers
   * between the host and the device. Moreover, it uses XRT buffer object as
   * memory buffers.
   * @param addr DMA address in the physical memory map. if 0, DMA engine for
   * AXI Stream is not instantiated but the memory map functionality will still
   * work.
   * @param hwparams Hardware-specific params
   */
  DMADataMover(const uint64_t addr,
               std::shared_ptr<HardwareParameters> hwparams);

  /**
   * @brief Default constructor
   *
   * The default constructor is deleted since the address is mandatory for the
   * DMA transfer.
   */
  DMADataMover() = delete;
  /**
   * @brief ~DMADataMover destructor method
   * Destroy the DMADataMover object.
   *
   */
  virtual ~DMADataMover();
  /**
   * @brief GetBuffer method
   * This method allocates a memory buffer. Depending on the MemoryType,
   * it allocates memory in a contiguous or memory region
   * (non-pageable) or non contiguous memory region depending on the Memory
   * typed past to the method. The memory can be mirrored with pageable memory
   * for its use in the host (or CPU).
   *
   * @param size Size in bytes of the buffer.
   *
   * @param type One of the values in the MemoryType enum class which can be one
   * of the following:
   * Dual (DIMM memory)
   * Cacheable (cache)
   * Host (Memory from the host)
   * Device (Memory from the device to be mapped)
   *
   * @param memory_bank Currently unused
   *
   * @return std::shared_ptr<IMemory>
   */
  std::shared_ptr<IMemory> GetBuffer(
      const size_t size, const int memory_bank = 0,
      const MemoryType type = MemoryType::Dual) override;
  /**
   * @brief Upload method
   * This method moves the data from the host to the device using a DMA engine.
   * This triggers the AXI Memory Map and AXI Stream Transactions under the
   * hood.
   *
   * @param mem XRTMemory instance to upload.
   *
   * @param size Size in bytes of data being uploaded in the memory device by
   * making use of the buffer.
   *
   * @param offset Offset in bytes where the device pointer should start
   *
   * @param exetype The execution type to use for the upload, this is either
   * sync (synchronous) or async (asynchronous) execution.
   *
   * @return Status
   */
  Status Upload(const std::shared_ptr<IMemory> mem, const size_t size,
                const size_t offset, const ExecutionType exetype) override;
  /**
   * @brief Download method
   *
   * @param mem IMemory instance to download.
   *
   * @param size Size in bytes of data being downloaded from the memory device
   * by making use of the buffer.
   *
   * @param offset Offset in bytes where the device pointer should start
   *
   * @param exetype The execution type to use for the download, this is either
   * sync (synchronous) or async (asynchronous) execution.
   *
   * @return Status
   */
  Status Download(const std::shared_ptr<IMemory> mem, const size_t size,
                  const size_t offset, const ExecutionType exetype) override;
  /**
   * @brief Sync method
   * Synchronizes data movements in case of asynchronous Upload/Download.
   *
   * @param type sync type. Depending on the transaction, it will trigger sync
   * @return Status
   */
  Status Sync(const SyncType type) override;
  /**
   * @brief GetStatus method
   * Returns the status of the data mover in terms of transactions.
   *
   * @return DeviceStatus
   */
  DeviceStatus GetStatus() override;

 private:
  /** Data Mover Parameters */
  std::unique_ptr<DataMoverParameters> data_mover_params_;
};
}  // namespace cynq
