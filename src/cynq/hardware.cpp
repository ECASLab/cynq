/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include <cynq/hardware.hpp>

#include <memory>

#include <cynq/ultrascale/hardware.hpp>

namespace cynq {
std::shared_ptr<IHardware> IHardware::Create(const HardwareArchitecture hw,
                                             const std::string& bitstream,
                                             const std::string& xclbin) {
  switch (hw) {
    case HardwareArchitecture::UltraScale:
      return std::make_shared<UltraScale>(bitstream, xclbin);
    default:
      return nullptr;
  }
}
}  // namespace cynq
