/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#include <atomic>  // NOLINT
#include <cynq/cynq.hpp>
#include <iostream>
#include <thread>  // NOLINT

/**
 * @example structures/execution-stream.cpp

 * This is a sample use case of the execution stream. It should not be
 * used directly by the factory. Instead, it is recommended to use it from
 * the IHardware instance given some possible restrictions. This example
 * is purely a test of the proof-of-concept.
 */

volatile std::atomic_int num{0};

cynq::Status dummy_function() {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "num: " << num.load() << std::endl;
  num++;
  return cynq::Status{};
}

int main(int, char **) {
  auto type = cynq::IExecutionGraph::Type::STREAM;
  auto stream = cynq::IExecutionGraph::Create(type, nullptr);

  cynq::IExecutionGraph::Function func = dummy_function;

  for (uint i = 0; i < 5; ++i) {
    stream->Add(func);
  }

  stream->Sync(2);
  std::cout << "Synchronised w.r.t. the third" << std::endl;

  stream->Sync();
  std::cout << "Synchronised w.r.t. the last fifth" << std::endl;

  stream->Add(func);
  stream->Sync();
  std::cout << "Synchronised w.r.t. the last sixth" << std::endl;

  return 0;
}
