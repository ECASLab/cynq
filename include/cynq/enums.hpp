/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#pragma once

enum class HardwareArchitecture {
	UltraScale
}

enum class SyncType {
	HostToDevice,
	DeviceToHost,
}

enum class StartMode {
	Once,
	Continuos
}

enum ExecutionType {
	Sync,
	Async
}

enum class DeviceStatus {
	Unkown,
	Done,
	Idle,
	Running,
	Error
}

enum class MemoryType {
	Dual,
	Cacheable,
	Host,
	Device
}
