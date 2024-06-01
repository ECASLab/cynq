/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#pragma once
#include <cynq/execution-graph.hpp>
#include <memory>

#include "cynq/enums.hpp"
#include "cynq/memory.hpp"
#include "cynq/status.hpp"

namespace cynq {

struct HardwareParameters;

/**
 * @brief Define an abstract representation of the data mover parameters
 * with some prefilled fields
 */
struct DataMoverParameters {
  /** HW Parameters */
  std::shared_ptr<HardwareParameters> hw_params_;
  /** Virtual destructor required for the inheritance */
  virtual ~DataMoverParameters() = default;
};

struct HardwareParameters;

/**
 * @brief Interface for standardising the API of DataMover for a specific
 * device:
 * XRTDataMover
 *
 */
class IDataMover {
 public:
  /**
   * @brief ~IDataMover destructor method
   * Destroy the IDataMover object.
   *
   */
  virtual ~IDataMover() = default;
  /**
   * @brief Type
   * Type of runtime supported by the IDataMover.
   *
   */
  enum Type {
    /** No runtime */
    None = 0,
    /** DMA-based runtime */
    DMA,
    /** XRT-based runtime */
    XRT
  };
  /**
   * @brief GetBuffer method
   * This method allocates a memory buffer. Depending on the MemoryType,
   * it allocates memory in a contiguous or memory region
   * (non-pageable) or non contiguous memory region depending on the Memory
   * typed past to the method. The memory can be mirrored with pageable
   * memory for its use in the host (or CPU).
   *
   * @param size Size in bytes of the buffer.
   *
   * @param type One of the values in the MemoryType enum class which can be
   * one of the following: Dual (DIMM memory) Cacheable (cache) Host (Memory
   * from the host) Device (Memory from the device to be mapped)
   *
   * @param memory_bank Memory bank corresponding to the memory to be
   * allocated. It is used for XRT-based data allocators.
   *
   * @return std::shared_ptr<IMemory>
   */
  virtual std::shared_ptr<IMemory> GetBuffer(
      const size_t size, const int memory_bank = 0,
      const MemoryType type = MemoryType::Dual) = 0;
  /**
   * @brief Upload method
   * This method moves the data from the host to the device using a DMA engine.
   * This triggers the AXI Memory Map and AXI Stream Transactions under the
   * hood.
   *
   * @param mem IMemory instance to upload.
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
  virtual Status Upload(const std::shared_ptr<IMemory> mem, const size_t size,
                        const size_t offset, const ExecutionType exetype) = 0;

  /**
   * @brief Upload method (asynchronous)
   * Please, refer to IDataMover::Upload for reference. This overload
   * performs an asynchronous execution of the function based on a graph
   * of operations. It returns as soon as the operation is scheduled
   *
   * @param graph Execution graph to execute on. If nullptr is passed, the
   * execution will be synchronous.
   *
   * @param mem IMemory instance to upload.
   *
   * @param size Size in bytes of data being uploaded in the memory device by
   * making use of the buffer.
   *
   * @param offset Offset in bytes where the device pointer should start
   *
   * @param exetype The execution type to use for the upload, this is either
   * sync (synchronous) or async (asynchronous) execution. In case of async
   * execution type, a Sync call must be performed afterwards to get coherent
   * results.
   *
   * @return Status
   */
  virtual Status Upload(std::shared_ptr<IExecutionGraph> graph,
                        const std::shared_ptr<IMemory> mem, const size_t size,
                        const size_t offset, const ExecutionType exetype);

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
  virtual Status Download(const std::shared_ptr<IMemory> mem, const size_t size,
                          const size_t offset, const ExecutionType exetype) = 0;

  /**
   * @brief Download method (asynchronous)
   * Please, refer to IDataMover::Download for reference. This overload
   * performs an asynchronous execution of the function based on a graph
   * of operations. It returns as soon as the operation is scheduled
   *
   * @param graph Execution graph to execute on. If nullptr is passed, the
   * execution will be synchronous.
   *
   * @param mem IMemory instance to download.
   *
   * @param size Size in bytes of data being downloaded from the memory device
   * by making use of the buffer.
   *
   * @param offset Offset in bytes where the device pointer should start
   *
   * @param exetype The execution type to use for the download, this is either
   * sync (synchronous) or async (asynchronous) execution. In case of async
   * execution type, a Sync call must be performed afterwards to get coherent
   * results.
   *
   * @return Status
   */
  virtual Status Download(std::shared_ptr<IExecutionGraph> graph,
                          const std::shared_ptr<IMemory> mem, const size_t size,
                          const size_t offset, const ExecutionType exetype);

  /**
   * @brief Sync method
   * Synchronizes data movements in case of asynchronous Upload/Download.
   *
   * @param type sync type. Depending on the transaction, it will trigger sync
   * @return Status
   */
  virtual Status Sync(const SyncType type) = 0;

  /**
   * @brief Sync method (asynchronous)
   * Please, refer to IDataMover::Sync for reference. This overload
   * performs an asynchronous execution of the function based on a graph
   * of operations. It returns as soon as the operation is scheduled
   *
   * @param graph Execution graph to execute on. If nullptr is passed, the
   * execution will be synchronous.
   *
   * @param type sync type. Depending on the transaction, it will trigger sync
   *
   * @return Status
   */
  virtual Status Sync(std::shared_ptr<IExecutionGraph> graph,
                      const SyncType type);

  /**
   * @brief GetStatus method
   * Returns the status of the data mover in terms of transactions.
   *
   * @return DeviceStatus
   */

  virtual DeviceStatus GetStatus() = 0;

  /**
   * @brief Create method
   * Factory method used for creating specific subclasses of IDataMover.
   *
   * @param impl
   * Used for establishin if the object is dependent on a runtime, use None if
   * this is not the case.
   *
   * @param addr
   * A 64 bit unsigned integer representing the beginning address of the
   * IDataMover.
   *
   * @param hwparams
   * Hardware-specific parameters to configure the data mover and grab the
   * correct memory blocks
   *
   * @return std::shared_ptr<IDataMover>
   * This is a shared_ptr with reference counting, the type will depend
   * on the value of impl, the options are the following:
   * following:
   * XRT -> XRTDatamover
   * None -> nullptr
   *
   */
  static std::shared_ptr<IDataMover> Create(
      IDataMover::Type impl, const uint64_t addr,
      std::shared_ptr<HardwareParameters> hwparams);
};
}  // namespace cynq
