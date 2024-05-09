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

/*
   -- Overloaded operations with fixed implementation --
   These functions are agnostic and independent from the
   platform but still require some implementable componets
*/

Status IDataMover::Upload(std::shared_ptr<IExecutionGraph> graph,
                          const std::shared_ptr<IMemory> mem, const size_t size,
                          const size_t offset, const ExecutionType exetype) {
  Status st{};

  /* Check the stream */
  if (!graph) {
    return this->Upload(mem, size, offset, exetype);
  }

  /* Functor to execute  */
  IExecutionGraph::Function func = [&, mem, size, offset]() -> Status {
    return this->Upload(mem, size, offset, exetype);
  };

  /* Add function */
  st.retval = graph->Add(func);
  return st;
}

Status IDataMover::Download(std::shared_ptr<IExecutionGraph> graph,
                            const std::shared_ptr<IMemory> mem,
                            const size_t size, const size_t offset,
                            const ExecutionType exetype) {
  Status st{};

  /* Check the stream */
  if (!graph) {
    return this->Download(mem, size, offset, exetype);
  }

  /* Functor to execute  */
  IExecutionGraph::Function func = [&, mem, size, offset, exetype]() -> Status {
    return this->Download(mem, size, offset, exetype);
  };

  /* Add function */
  st.retval = graph->Add(func);
  return st;
}

Status IDataMover::Sync(std::shared_ptr<IExecutionGraph> graph,
                        const SyncType type) {
  Status st{};

  /* Check the stream */
  if (!graph) {
    return this->Sync(type);
  }

  /* Functor to execute  */
  IExecutionGraph::Function func = [&, type]() -> Status {
    return this->Sync(type);
  };

  /* Add function */
  st.retval = graph->Add(func);
  return st;
}
}  // namespace cynq
