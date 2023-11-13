/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */

#include <cynq/ultrascale/hardware.hpp>

#include <memory>
#include <string>

#include <cynq/accelerator.hpp>
#include <cynq/amd/accelerator.hpp>
#include <cynq/enums.hpp>
#include <cynq/hardware.hpp>
#include <cynq/status.hpp>
#include <cynq/xrt/datamover.hpp>

namespace cynq {
Status UltraScale::Reset() { return Status{}; }

std::shared_ptr<IDataMover> UltraScale::GetDataMover(const uint64_t address) {
  return IDataMover::Create(IDataMover::XRT, address);
}

std::shared_ptr<IAccelerator> UltraScale::GetAccelerator(
    const uint64_t address) {
  return IAccelerator::Create(IAccelerator::XRT, address);
}
}  // namespace cynq
