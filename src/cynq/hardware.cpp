/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include "cynq/hardware.hpp"

#include <memory>

#include "cynq/ultrascale/hardware.hpp"

std::shared_ptr<IHardware> IHardware::Create(const HardwareArchitecture hw,
                                             const std::string &bitstream,
                                             const std::string &xclbin) {
  switch (hw) {
    case hw::UltraScale:
      return std::make_shared<UltraScale>();
    default:
      return nullptr;
  }
}
