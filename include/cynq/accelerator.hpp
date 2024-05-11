/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#pragma once
#include <memory>
#include <string>

// cynq headers
#include <cynq/enums.hpp>
#include <cynq/execution-graph.hpp>
#include <cynq/memory.hpp>
#include <cynq/status.hpp>

namespace cynq {

struct HardwareParameters;

/**
 * @brief Define an abstract representation of the accelerator parameters
 * with some prefilled fields
 */
struct AcceleratorParameters {
  /** Virtual destructor required for the inheritance */
  virtual ~AcceleratorParameters() = default;
};

/**
 * @brief Interface for standardising the API for any Accelerator device:
 * XRTAccelerator
 *
 */
class IAccelerator {
 public:
  /**
   * @brief ~IAccelerator destructor method
   * Destroy the IAccelerator object.
   *
   */
  virtual ~IAccelerator() = default;
  /**
   * @brief Type
   * Type of runtime supported by the IAccelerator.
   *
   */
  enum Type {
    /** No runtime */
    None = 0,
    /** MMIO runtime: compatible with ZYNQ and Vivado workflows */
    MMIO,
    /** XRT kernel runtime: compatible with Vitis and Alveo workflows */
    XRT
  };

  /**
   * @brief Start method
   * This method starts the accelerator in either once or continuous mode (with
   * the autorestart). Under the hood, this writes the control register to turn
   * on the accelerator with/without the autorestart bit.
   *
   * @param mode One of the values in the StartMode enum class
   * present in the enums.hpp file.
   *
   * @return Status
   */
  virtual Status Start(const StartMode mode) = 0;

  /**
   * @brief Start method (asynchronous)
   * Please, refer to IAccelerator::Start for reference. This overload
   * performs an asynchronous execution of the function based on a graph
   * of operations. It returns as soon as the operation is scheduled
   *
   * @param mode One of the values in the StartMode enum class
   * present in the enums.hpp file.
   * @param graph Execution graph to execute on. If nullptr is passed, the
   * execution will be synchronous.
   *
   * @return Status. It fills the retval field with the NodeID of the
   * execution graph.
   */
  virtual Status Start(std::shared_ptr<IExecutionGraph> graph,
                       const StartMode mode);

  /**
   * @brief Stop method
   * This asynchronously turns off the accelerator by removing the autorestart
   * and start bits from the control registers. Please, note that the
   * accelerator will turn off once it finishes its current task.
   *
   * @return Status
   */
  virtual Status Stop() = 0;

  /**
   * @brief Stop method (asynchronous)
   * Please, refer to IAccelerator::Stop for reference. This overload
   * performs an asynchronous execution of the function based on a graph
   * of operations. It returns as soon as the operation is scheduled
   *
   * @param graph Execution graph to execute on. If nullptr is passed, the
   * execution will be synchronous.
   *
   * @return Status. It fills the retval field with the NodeID of the
   * execution graph.
   */
  virtual Status Stop(std::shared_ptr<IExecutionGraph> graph);

  /**
   * @brief Sync method
   * This synchronises the execution and wait until the kernel/accelerator
   * finishes its execution.
   *
   * @return Status
   */
  virtual Status Sync() = 0;

  /**
   * @brief Sync method (asynchronous)
   * Please, refer to IAccelerator::Sync for reference. This overload
   * performs an asynchronous execution of the function based on a graph
   * of operations. It returns as soon as the operation is scheduled
   *
   * @param graph Execution graph to execute on. If nullptr is passed, the
   * execution will be synchronous.
   *
   * @return Status. It fills the retval field with the NodeID of the
   * execution graph.
   */
  virtual Status Sync(std::shared_ptr<IExecutionGraph> graph);

  /**
   * @brief GetStatus method
   * This returns the accelerator state by using the DeviceStatus. This reads
   * the control register flags.
   *
   * @return DeviceStatus
   */
  virtual DeviceStatus GetStatus() = 0;

  /**
   * @brief Create method
   * Factory method used for creating specific subclasses of IAccelerator.
   *
   * @param impl
   * Used for establishing if the object is dependent on a runtime, use None if
   * this is not the case.
   *
   * @param addr
   * A 64 bit unsigned integer establishing the address from which the address
   * space of the accelerator starts.
   *
   * @return std::shared_ptr<IAccelerator>
   * This is a shared_ptr with reference counting, the type will depend
   * on the value of impl, the options are the following:
   * XRT -> XRTAccelerator
   * MMIO -> MMIOAccelerator
   * None -> nullptr
   *
   */
  static std::shared_ptr<IAccelerator> Create(IAccelerator::Type impl,
                                              const uint64_t addr);
  /**
   * @brief Create method (overload)
   * Factory method used for creating specific subclasses of IAccelerator.
   *
   * @param impl
   * Used for establishing if the object is dependent on a runtime, use None if
   * this is not the case.
   *
   * @param kernelname
   * Name of the kernel in case of using XRT runtime and the Vitis workflow.
   * Use this factory only in case of being in a ZYNQ with Vitis workflow or
   * in the Alveo.
   *
   * @param hwparams
   * Hardware parameters required for creating the accelerator
   *
   * @return std::shared_ptr<IAccelerator>
   * This is a shared_ptr with reference counting, the type will depend
   * on the value of impl, the options are the following:
   * XRT -> XRTAccelerator
   * None -> nullptr
   *
   */
  static std::shared_ptr<IAccelerator> Create(
      IAccelerator::Type impl, const std::string &kernelname,
      const std::shared_ptr<HardwareParameters> hwparams);

  /**
   * @brief Write method
   * Performs a write operation to the accelerator through a register.
   *
   * @tparam T
   * Datatype used as the individual unit of information being written to the
   * device.
   *
   * @param address
   * Address of the accelerator.
   *
   * @param data
   * Raw pointer of type T used by the register to access the data.
   *
   * @param elements
   * Number of elements being written to the device. Defaults to one
   *
   * @return Status
   */
  template <typename T>
  Status Write(const uint64_t address, const T *data,
               const size_t elements = 1) {
    return this->WriteRegister(address, reinterpret_cast<const uint8_t *>(data),
                               elements * sizeof(T));
  }

  /**
   * @brief Write method (asynchronous)
   * Performs a write operation to the accelerator through a register in
   * an execution graph. It returns as soon as the instruction is scheduled
   *
   * @tparam T
   * Datatype used as the individual unit of information being written to the
   * device.
   *
   * @param graph Execution graph to execute on. If nullptr is passed, the
   * execution will be synchronous.
   *
   * @param address
   * Address of the accelerator.
   *
   * @param data
   * Raw pointer of type T used by the register to access the data.
   *
   * @param elements
   * Number of elements being written to the device. Defaults to one
   *
   * @return Status. It fills the retval field with the NodeID within the
   * execution graph
   */
  template <typename T>
  Status Write(std::shared_ptr<IExecutionGraph> graph, const uint64_t address,
               const T *data, const size_t elements = 1) {
    return this->WriteRegister(graph, address,
                               reinterpret_cast<const uint8_t *>(data),
                               elements * sizeof(T));
  }

  /**
   * @brief Read method
   * Performs a write operation to the accelerator through a register.
   *
   * @tparam T
   * Datatype used as the individual unit of information being read from the
   * device.
   *
   * @param address
   * Address of the accelerator.
   *
   * @param data
   * Raw pointer of type T used by the register to access the data.
   *
   * @param elements
   * Number of elements being read from the device. Defaults to one
   *
   * @return Status
   */
  template <typename T>
  Status Read(const uint64_t address, T *data, const size_t elements = 1) {
    return this->ReadRegister(address, reinterpret_cast<uint8_t *>(data),
                              elements * sizeof(T));
  }

  /**
   * @brief Read method (asynchronous)
   * Performs a write operation to the accelerator through a register in
   * an execution graph. It returns as soon as the instruction is scheduled
   *
   * @tparam T
   * Datatype used as the individual unit of information being read from the
   * device.
   *
   * @param graph Execution graph to execute on. If nullptr is passed, the
   * execution will be synchronous.
   *
   * @param address
   * Address of the accelerator.
   *
   * @param data
   * Raw pointer of type T used by the register to access the data.
   *
   * @param elements
   * Number of elements being read from the device. Defaults to one
   *
   * @return Status. It fills the retval field with the NodeID within the
   * execution graph
   */
  template <typename T>
  Status Read(std::shared_ptr<IExecutionGraph> graph, const uint64_t address,
              T *data, const size_t elements = 1) {
    return this->ReadRegister(graph, address, reinterpret_cast<uint8_t *>(data),
                              elements * sizeof(T));
  }

  /**
   * @brief Get the memory bank ID
   *
   * It corresponds to the argument memory argument for affinity. It is useful
   * for assigning memory banks to the DataMovers before requesting any memory.
   *
   * It is only used by Vitis and Alveo workflows
   *
   * @param pos memory bank position within the kernel
   *
   * @return the memory bank
   */
  virtual int GetMemoryBank(const uint pos) = 0;

  /**
   * @brief Attach an argument
   * Performs an attachment of the argument and the respective pointer. If the
   * index/address to attach receives a nullptr, it gets detached.
   *
   * @tparam T
   * Datatype used as the individual unit of information being written to the
   * device.
   *
   * @param index Argument position/address (Ultrascale) of the argument to set
   *
   * @param data Raw pointer of type T used by the register to access the data.
   * The ownership is borrowed and its existence must be guaranteed.
   *
   * @param access Access type of the register (read-only, write-only,
   * read-write). Defaults to Write-Only (autodetected and unused in Alveo or
   * Vitis-based).
   *
   * @param elements Number of elements being written to the device. Defaults to
   * one
   *
   * @return Status
   */
  template <typename T>
  Status Attach(const uint64_t index, T *data,
                const RegisterAccess access = RegisterAccess::WO,
                const size_t elements = 1) {
    return this->AttachRegister(index, reinterpret_cast<uint8_t *>(data),
                                access, elements * sizeof(T));
  }

  /**
   * @brief Overload of the Attach<T>()
   *
   * See Attach()
   *
   */
  template <typename T>
  Status Attach(const uint64_t index, T *data, const size_t elements,
                const RegisterAccess access = RegisterAccess::WO) {
    return this->AttachRegister(index, reinterpret_cast<uint8_t *>(data),
                                access, elements * sizeof(T));
  }

  /**
   * @brief Attach a memory argument
   * Performs an attachment of the argument and the respective pointer.
   * The use of this overload for IMemory buffers is highly recommended.
   *
   * @param addr Argument address to set the memory address. In the case of
   * Alveo or Vitis-based workflows, it is the argument index.
   *
   * @param mem Memory buffer to attach to the argument
   *
   * @return Status
   */
  virtual Status Attach(const uint64_t addr, std::shared_ptr<IMemory> mem) = 0;

 protected:
  /**
   * @brief Opaque Write Register method
   * Writes to the register of the accelerator.
   *
   * @param address an unsigned integer of 64 bits representing an address.
   *
   * @param data a pointer to an unsigned 8 bits variable which holds the
   * data to write to the register.
   *
   * @param size size in bytes of the data to write.
   *
   * @return Status
   */
  virtual Status WriteRegister(const uint64_t address, const uint8_t *data,
                               const size_t size) = 0;
  /**
   * @brief Opaque Read Register method
   *
   * @param address an unsigned integer of 64 bits representing an address.
   *
   * @param data a pointer to an unsigned 8 bits variable which holds the
   * data to read from the register.
   *
   * @param size size in bytes of the data to read.
   *
   * @return Status
   */
  virtual Status ReadRegister(const uint64_t address, uint8_t *data,
                              const size_t size) = 0;

  /**
   * @brief Write Register method (asynchronous)
   *
   * @param graph Execution graph to execute on. If nullptr is passed, the
   * execution will be synchronous.
   *
   * @param address an unsigned integer of 64 bits representing an address.
   *
   * @param data a pointer to an unsigned 8 bits variable which holds the
   * data to write to the register.
   *
   * @param size size in bytes of the data to write.
   *
   * @return Status
   */
  virtual Status WriteRegister(std::shared_ptr<IExecutionGraph> graph,
                               const uint64_t address, const uint8_t *data,
                               const size_t size);
  /**
   * @brief Read Register method (asynchronous)
   *
   * @param graph Execution graph to execute on. If nullptr is passed, the
   * execution will be synchronous.
   *
   * @param address an unsigned integer of 64 bits representing an address.
   *
   * @param data a pointer to an unsigned 8 bits variable which holds the
   * data to read from the register.
   *
   * @param size size in bytes of the data to read.
   *
   * @return Status
   */
  virtual Status ReadRegister(std::shared_ptr<IExecutionGraph> graph,
                              const uint64_t address, uint8_t *data,
                              const size_t size);

  /**
   * @brief Opaque Attach Register method
   *
   * @param index index of the argument to set
   *
   * @param data a pointer to an unsigned 8 bits variable which holds the
   * data to read from the register.
   *
   * @param access Access type of the register (read-only, write-only,
   * read-write).
   *
   * @param size size in bytes of the data to read.
   *
   * @return Status
   */
  virtual Status AttachRegister(const uint64_t index, uint8_t *data,
                                const RegisterAccess access,
                                const size_t size) = 0;
};
}  // namespace cynq
