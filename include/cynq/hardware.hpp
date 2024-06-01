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

#include "cynq/accelerator.hpp"
#include "cynq/datamover.hpp"
#include "cynq/enums.hpp"
#include "cynq/execution-graph.hpp"
#include "cynq/status.hpp"

namespace cynq {
/**
 * @brief Define an abstract representation of the hardware parameters
 * with some prefilled fields
 */
struct HardwareParameters {
  /** Bitstream file path */
  std::string bitstream_file;
  /** XCLbin file path */
  std::string xclbin_file;
  /** Virtual destructor required for the inheritance */
  virtual ~HardwareParameters() = default;
};

/**
 * @brief Interface for standardising the API of Hardware Devices:
 * - no inheritors -
 */
class IHardware {
 public:
  /**
   * @brief ~IHardware destructor method
   * Destroy the IHardware object.
   *
   */
  virtual ~IHardware() = default;
  /**
   * @brief Type
   * Type of runtime supported by the IHardware.
   *
   */
  enum Type {
    /** No runtime */
    None = 0,
    /** Xilinx runtime */
    XRT
  };
  /**
   * @brief Reset method
   * Sets the IHardware instance to its initial state.
   *
   * @return Status
   */
  virtual Status Reset() = 0;
  /**
   * @brief GetDataMover method
   * Used for accessing the IDataMover instance of
   * IHardware inheritors for decoupling the DataMovement separating the
   * hardware logic from the data movement logic.
   *
   * @param address a unsigned integer of 64 bits representing an address.
   * In the case of ZYNQ boards, it corresponds to the base address of the
   * accelerator BAR (Bank Address Register). In the case of Alveo boards,
   * it is unused.
   *
   * @return std::shared_ptr<IDataMover>
   * Returns an IDataMover pointer with reference counting. It should be
   * thread-safe.
   *
   */
  virtual std::shared_ptr<IDataMover> GetDataMover(const uint64_t address) = 0;
  /**
   * @brief GetAccelerator method
   * IAccelerator instance of IAccelerator inheritors separating the hardware
   * logic from the specific logic of the accelerator.
   *
   * @param address a unsigned integer of 64 bits representing an address.
   * In the case of ZYNQ boards, it corresponds to the base address of the
   * accelerator BAR (Bank Address Register). In the case of Alveo boards,
   * it is preferrable to use the GetAccelerator(const std::string &) overload.
   *
   * @return std::shared_ptr<IAccelerator>
   * Returns an IAccelerator pointer with reference counting. It should be
   * thread-safe.
   *
   */
  virtual std::shared_ptr<IAccelerator> GetAccelerator(
      const uint64_t address) = 0;

  /**
   * @brief GetAccelerator method
   * IAccelerator instance of IAccelerator inheritors separating the hardware
   * logic from the specific logic of the accelerator.
   *
   * @param kernelname string that contains the kernel name to launch. It is
   * used by the Vitis and Alveo workflows. It is not implemented in Vivado
   * workflows
   *
   * @return std::shared_ptr<IAccelerator>
   * Returns an IAccelerator pointer with reference counting. It should be
   * thread-safe.
   *
   */
  virtual std::shared_ptr<IAccelerator> GetAccelerator(
      const std::string &kernelname) = 0;

  /**
   * @brief GetExecutionStream
   *
   * This method is a factory method to obtain an execution stream compatible
   * with the hardware implementation. By default, it returns a new execution
   * stream similar to the CUDA Stream, which is a queue-based scheduler to
   * manage synchronism.
   *
   * @param name name of the stream for debugging purposes
   *
   * @param type implementation for the execution graph. By default, it is
   * STREAM
   *
   * @param config configurations of the execution graph. By default, it is
   * empty.
   *
   * @return std::shared_ptr<IExecutionGraph>
   * Returns an execution graph instance compatible with the API of the
   * interface IExecutionGraph
   */
  virtual std::shared_ptr<IExecutionGraph> GetExecutionStream(
      const std::string &name,
      const IExecutionGraph::Type type = IExecutionGraph::Type::STREAM,
      const std::shared_ptr<ExecutionGraphParameters> params = nullptr);

  /**
   * @brief Create method
   * Factory method to create a hardware-specific subclasses for accelerators
   * and data movers.
   *
   * @param hw One of the values in the HardwareArchitecture enum class
   * present in the enums.hpp file that should correspond to the device being
   * used.
   *
   * @param bitstream string that represents the name of the file
   * with the bitstream. It is used for normal Vivado flow in ZYNQ boards.
   * In Alveo, it is neglected.
   *
   * @param xclbin string that represents the name of the xclbin file.
   * Used for Vitis and Alveo workflows
   *
   * @return std::shared_ptr<IHardware>
   * Returns an IAccelerator pointer with reference counting. It should be
   * thread-safe.
   *
   */
  static std::shared_ptr<IHardware> Create(const HardwareArchitecture hw,
                                           const std::string &bitstream,
                                           const std::string &xclbin);

  /**
   * @brief Create method
   * Factory method to create a hardware-specific subclasses for accelerators
   * and data movers.
   *
   * @param hw One of the values in the HardwareArchitecture enum class
   * present in the enums.hpp file that should correspond to the device being
   * used.
   *
   * @param config string that represents the name of the file
   * with the bitstream in the case of Ultrascale or xclbin for Vitis and
   * Alveo workflows
   *
   * @return std::shared_ptr<IHardware>
   * Returns an IAccelerator pointer with reference counting. It should be
   * thread-safe.
   *
   */
  static std::shared_ptr<IHardware> Create(const HardwareArchitecture hw,
                                           const std::string &config);
};
}  // namespace cynq
