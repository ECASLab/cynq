/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023-2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include <cynq/accelerator.hpp>
#include <cynq/execution-graph.hpp>
#include <cynq/mmio/accelerator.hpp>
#include <cynq/xrt/accelerator.hpp>
#include <memory>

namespace cynq {
std::shared_ptr<IAccelerator> IAccelerator::Create(IAccelerator::Type impl,
                                                   const uint64_t addr) {
  switch (impl) {
    case IAccelerator::Type::MMIO:
      return std::make_shared<MMIOAccelerator>(addr);
    default:
      return nullptr;
  }
}

std::shared_ptr<IAccelerator> IAccelerator::Create(
    IAccelerator::Type impl, const std::string &addr,
    const std::shared_ptr<HardwareParameters> params) {
  switch (impl) {
    case IAccelerator::Type::XRT:
      return std::make_shared<XRTAccelerator>(addr, params);
    default:
      return nullptr;
  }
}

/*
   -- Overloaded operations with fixed implementation --
   These functions are agnostic and independent from the
   platform but still require some implementable components
*/

Status IAccelerator::Start(std::shared_ptr<IExecutionGraph> graph,
                           const StartMode mode) {
  Status st{};

  /* Check the stream */
  if (!graph) {
    return this->Start(mode);
  }

  /* Functor to execute  */
  IExecutionGraph::Function func = [&, mode]() -> Status {
    return this->Start(mode);
  };

  /* Add function */
  st.retval = graph->Add(func);
  return st;
}

Status IAccelerator::Stop(std::shared_ptr<IExecutionGraph> graph) {
  Status st{};

  /* Check the stream */
  if (!graph) {
    return this->Stop();
  }

  /* Functor to execute  */
  IExecutionGraph::Function func = [&]() -> Status { return this->Stop(); };

  /* Add function */
  st.retval = graph->Add(func);
  return st;
}

Status IAccelerator::Sync(std::shared_ptr<IExecutionGraph> graph) {
  Status st{};

  /* Check the stream */
  if (!graph) {
    return this->Sync();
  }

  /* Functor to execute  */
  IExecutionGraph::Function func = [&]() -> Status { return this->Sync(); };

  /* Add function */
  st.retval = graph->Add(func);
  return st;
}

Status IAccelerator::WriteRegister(std::shared_ptr<IExecutionGraph> graph,
                                   const uint64_t address, const uint8_t *data,
                                   const size_t size) {
  Status st{};

  /* Check the stream */
  if (!graph) {
    return this->WriteRegister(address, data, size);
  }

  /* Functor to execute  */
  IExecutionGraph::Function func = [&, address, data, size]() -> Status {
    return this->WriteRegister(address, data, size);
  };

  /* Add function */
  st.retval = graph->Add(func);
  return st;
}

Status IAccelerator::ReadRegister(std::shared_ptr<IExecutionGraph> graph,
                                  const uint64_t address, uint8_t *data,
                                  const size_t size) {
  Status st{};

  /* Check the stream */
  if (!graph) {
    return this->ReadRegister(address, data, size);
  }

  /* Functor to execute  */
  IExecutionGraph::Function func = [&, address, data, size]() -> Status {
    return this->ReadRegister(address, data, size);
  };

  /* Add function */
  st.retval = graph->Add(func);
  return st;
}

}  // namespace cynq
