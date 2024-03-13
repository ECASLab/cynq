/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#pragma once

namespace cynq {
/**
 * @brief HardwareArchitecture
 * Defines the architecture of the Create method. This is used by classes
 * that implement the IHardware interface.
 */
enum class HardwareArchitecture {
  /** For ultra scale xilinx devices */
  UltraScale,
  /** For Alveo cards */
  Alveo
};

/**
 * @brief SyncType
 * Declares the orientation of data synchronization in the Sync method.
 * This is used by any class that implements the IMemory interface.
 */
enum class SyncType {
  /** Synchronization from host (CPU) to the device (FPGA) */
  HostToDevice,
  /** Synchronization from device (FPGA) to the host (CPU) */
  DeviceToHost
};

/**
 * @brief StartMode
 * Mode for the Start method of IAccelerator. This is used by any class
 * that implements the IAcelerator interface.
 */
enum class StartMode {
  /** Mode is Once at initialiization */
  Once,
  /** Mode is Continuos at initialization */
  Continuous
};

/**
 * @brief ExecutionType
 * Style of execution for the API. This is used by any class that implements
 * the IDataMover interface.
 */
enum class ExecutionType {
  /* Syncrhonous style of execution for IDataMover **/
  Sync,
  /* Asyncrhonous style of execution for IDataMover **/
  Async
};

/**
 * @brief DeviceStatus
 * Possible state flags used to determine state of the device,
 * these are used by any class that implements the IAccelerator
 * and IDataMover interfaces as the return of GetStatus().
 */
enum class DeviceStatus {
  /** Uknown status for IAccelerator **/
  Unknown = -1,
  /** Done status for IAccelerator **/
  Done,
  /** Idle status for IAccelerator **/
  Idle,
  /** Running status for IAccelerator **/
  Running,
  /** Error status for IAccelerator **/
  Error
};

/**
 * @brief MemoryType
 * Types of memory for a IDataMover. This is used by any class that implements
 * IDataMover in the GetBuffer() method.
 */
enum class MemoryType {
  /** GetBuffer receives dual as memory type. */
  Dual,
  /** GetBuffer receives cacheable as memory type. */
  Cacheable,
  /** GetBuffer receives host as memory type. */
  Host,
  /** GetBuffer receives device as memory type. */
  Device
};

/**
 * @brief RegisterAccess
 * Enumerator of the types of registers in terms of the access
 */
enum class RegisterAccess {
  /** Read-only access */
  RO,
  /** Write-only access */
  WO,
  /** Read-write access */
  RW,
  /** Auto-detection: for Alveo/Vitis */
  Auto,
};
}  // namespace cynq
