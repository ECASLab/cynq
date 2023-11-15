/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include <cynq/datamover.hpp>

#include <memory>

#include <cynq/xrt/datamover.hpp>

namespace cynq {
std::shared_ptr<IDataMover> IDataMover::Create(IDataMover::Type impl,
                                               const uint64_t /*addr*/) {
  switch (impl) {
    case IDataMover::Type::XRT:
      return std::make_shared<XRTDataMover>();
    default:
      return nullptr;
  }
}
}  // namespace cynq
