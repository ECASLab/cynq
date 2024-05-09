/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include <cynq/alveo/hardware.hpp>
#include <cynq/hardware.hpp>
#include <cynq/ultrascale/hardware.hpp>
#include <memory>

namespace cynq {
std::shared_ptr<IHardware> IHardware::Create(const HardwareArchitecture hw,
                                             const std::string& bitstream,
                                             const std::string& xclbin) {
  switch (hw) {
    case HardwareArchitecture::UltraScale:
      return std::make_shared<UltraScale>(bitstream, xclbin);
    case HardwareArchitecture::Alveo:
      return std::make_shared<Alveo>(bitstream, xclbin);
    default:
      return nullptr;
  }
}

std::shared_ptr<IHardware> IHardware::Create(const HardwareArchitecture hw,
                                             const std::string& config) {
  switch (hw) {
    case HardwareArchitecture::UltraScale:
      return std::make_shared<UltraScale>(config,
                                          EXAMPLE_KRIA_DEFAULT_XCLBIN_LOCATION);
    case HardwareArchitecture::Alveo:
      return std::make_shared<Alveo>("", config);
    default:
      return nullptr;
  }
}

std::shared_ptr<IExecutionGraph> IHardware::GetExecutionStream(
    const std::string& name, const IExecutionGraph::Type type,
    const std::shared_ptr<ExecutionGraphParameters> params) {
  std::shared_ptr<ExecutionGraphParameters> output_params = nullptr;

  if (!params) {
    output_params = std::make_shared<ExecutionGraphParameters>();
  } else {
    output_params = params;
  }

  output_params->name = name;
  return IExecutionGraph::Create(type, output_params);
}

}  // namespace cynq
