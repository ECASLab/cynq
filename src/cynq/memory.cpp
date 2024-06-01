/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#include <cynq/memory.hpp>
#include <cynq/xrt/memory.hpp>
#include <memory>

namespace cynq {
std::shared_ptr<IMemory> IMemory::Create(IMemory::Type impl,
                                         const std::size_t size,
                                         uint8_t* hostptr, uint8_t* devptr,
                                         void* moverptr) {
  switch (impl) {
    case IMemory::Type::XRT:
      return std::make_shared<XRTMemory>(size, hostptr, devptr, moverptr);
    default:
      return nullptr;
  }
}

Status IMemory::Sync(std::shared_ptr<IExecutionGraph> graph,
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
