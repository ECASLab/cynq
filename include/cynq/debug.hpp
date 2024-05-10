/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */

#pragma once

#include <iostream>

namespace cynq {
/**
 * Enumerator listing the different log levels
 */
enum class LOG {
  /** Unrecoverable errors */
  ERROR = 0,
  /** Recoverable errors */
  WARN = 1,
  /** Information logs */
  INFO = 2,
  /** Debud information */
  DEBUG = 3
};

template <typename T>
void CYNQ_LOG(T value) {
  std::cout << value << std::endl;
}

template <typename T, typename... Args>
void CYNQ_LOG(T value, Args... args) {
  std::cout << value << " ";
  CYNQ_LOG(args...);
}

/**
 * @brief Debugging function
 *
 * @tparam T first argument type
 * @tparam Args types
 *
 * @param log log level in cynq::LOG
 * @param value first argument value
 * @param value other argument values
 */
template <typename T, typename... Args>
void CYNQ_DEBUG(const LOG log, T value, Args... args) {
#ifdef DEBUG_MODE
  switch (log) {
    case LOG::ERROR: {
#if DEBUG_MODE >= 0
      std::cout << "[CYNQ ERROR]: ";
      std::cout << value << " ";
      CYNQ_LOG(args...);
#endif
    } break;
    case LOG::WARN: {
#if DEBUG_MODE >= 1
      std::cout << "[CYNQ WARN]: ";
      std::cout << value << " ";
      CYNQ_LOG(args...);
#endif
    } break;
    case LOG::INFO: {
#if DEBUG_MODE >= 2
      std::cout << "[CYNQ INFO]: ";
      std::cout << value << " ";
      CYNQ_LOG(args...);
#endif
    } break;
    default: {
#if DEBUG_MODE >= 3
      std::cout << "[CYNQ DEBUG]: ";
      std::cout << value << " ";
      CYNQ_LOG(args...);
#endif
    } break;
  }
#endif
}

}  // namespace cynq
