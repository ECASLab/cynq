/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include <cynq/accelerator.hpp>

#include <memory>

#include <cynq/amd/accelerator.hpp>

namespace cynq {
std::shared_ptr<IAccelerator> IAccelerator::Create(IAccelerator::Type impl,
                                                   const uint64_t /*addr*/) {
  switch (impl) {
    case IAccelerator::Type::XRT:
      return std::make_shared<XRTAccelerator>();
    default:
      return nullptr;
  }
}
}  // namespace cynq
