/**
 *  Templated BST - profiling instrumentation
 *  MIT License - Copyright 2024 - Luis G. Leon-Vega.
 */

/**
 * @file time.hpp
 */
#pragma once
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#define macstr(s) #s

typedef std::chrono::high_resolution_clock::time_point time_point_t;
#define INIT_PROFILER(NAME) profiler NAME{};
#define GET_PROFILE_INSTANCE(NAME, PROFILER) \
  profile_node *NAME = (PROFILER).create(macstr(NAME));
#define START_PROFILE(NAME, PROFILER, ITER)          \
  profile_node *NAME = (PROFILER).create(macstr(NAME)); \
  for (size_t i{0}; i < (ITER); ++i) {
#define END_PROFILE(NAME) \
  (NAME)->tick();         \
  }

/* Profiler instance */
class profile_node {
 public:
  std::string name;
  double average;
  double stddev;
  std::vector<double> samples;
  time_point_t tlast;

  profile_node(std::string name_)
      : name{name_},
        average{0},
        stddev{0},
        tlast{std::chrono::high_resolution_clock::now()} {}
  void tick() {
    time_point_t tnow = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span =
        std::chrono::duration_cast<std::chrono::duration<double>>(tnow - tlast);
    samples.push_back(time_span.count());
    tlast = tnow;
  }
  void reset() { tlast = std::chrono::high_resolution_clock::now(); }
  friend std::ostream &operator<<(std::ostream &os, profile_node &pn) {
    // Compute the average
    double gsum = std::reduce(pn.samples.begin(), pn.samples.end());
    double average = gsum / pn.samples.size();

    // Compute the sum of the square differences
    std::transform(
        pn.samples.cbegin(), pn.samples.cend(), pn.samples.begin(),
        [&](double val) { return ((val - average) * (val - average)); });
    double gsumdiff = std::reduce(pn.samples.begin(), pn.samples.end());
    double stddev = std::sqrt(gsumdiff / pn.samples.size());

    os << "-- " << pn.name << " --"
       << " (AVG: " << average << ", STD: " << stddev
       << ", IT:" << pn.samples.size() << ")";
    return os;
  }
};

class profiler {
  std::vector<std::unique_ptr<profile_node>> profilers;

 public:
  /* Default */
  profiler(){};

  /* Create a new instance */
  profile_node *create(std::string name) {
    profilers.push_back(std::make_unique<profile_node>(name));
    return profilers[profilers.size() - 1].get();
  }
  /* Print instance */
  friend std::ostream &operator<<(std::ostream &os, const profiler &p) {
    os << "Printing profile results: \n";
    for (size_t i{0}; i < p.profilers.size(); ++i) {
      os << *(p.profilers[i].get()) << "\n";
    }
    return os;
  }
};
