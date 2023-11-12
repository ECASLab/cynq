/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include "cynq/memory.hpp"

#include <memory>

#include "cynq/xrt/memory.hpp"

std::shared_ptr<IMemory> IMemory::Create(IMemory::Type impl,
                                         const std::size_t size,
                                         uint8_t* hostptr, uint8_t* devptr) {
  switch (impl) {
    case impl::XRT:
      return std::make_shared<XRTMemory>();
    default:
      return nullptr;
  }
}
