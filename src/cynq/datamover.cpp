/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include <cynq/datamover.hpp>
#include <cynq/dma/datamover.hpp>
#include <cynq/xrt/datamover.hpp>
#include <memory>

namespace cynq {
std::shared_ptr<IDataMover> IDataMover::Create(
    IDataMover::Type impl, const uint64_t addr,
    std::shared_ptr<HardwareParameters> hwparams) {
  switch (impl) {
    case IDataMover::Type::DMA:
      return std::make_shared<DMADataMover>(addr, hwparams);
    case IDataMover::Type::XRT:
      return std::make_shared<XRTDataMover>(addr, hwparams);
    default:
      return nullptr;
  }
}
}  // namespace cynq
