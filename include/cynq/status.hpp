/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2023
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 *         Diego Arturo Avila Torres <diego.avila@uned.cr>
 *
 */
#pragma once

#include <string>

namespace cynq {
/**
 * @brief Structure to define the return characteristics of each function
 *
 * It includes a code and a description that works to track errors
 */
struct Status {
  /** Error codes */
  enum {
    OK = 0,            /** OK Status */
    FILE_ERROR,        /** File error that can be read or write */
    INVALID_PARAMETER, /** Invalid argument or parameter. i.e. nullptr */
    /** Incompatible parameter that it is not supported
                              by a function */
    INCOMPATIBLE_PARAMETER,
    CONFIGURATION_ERROR,  /** Configuration error*/
    REGISTER_IO_ERROR,    /** Register MMIO error */
    NOT_IMPLEMENTED,      /** Not implemented error */
    MEMBER_ABSENT,        /** Missing member */
    RESOURCE_BUSY,        /** Busy */
    EXECUTION_FAILED,     /** Cannot execute the IP */
    REGISTER_NOT_ALIGNED, /** Issues with alignment when writing a reg */
  };

  int code;        /** Code of the error */
  int retval;      /** Auxiliar data coming from user */
  std::string msg; /** Description of the error */

  /**
   * @brief Construct a new Status object
   *
   * It default the error to be 0 or OK
   */
  Status() noexcept : code{0}, retval{0} {}

  /**
   * @brief Construct a new Status object
   *
   * It defines the constructor to define a custom code and description
   *
   * @param code code of the error
   * @param msg description
   */
  Status(const int code, const std::string &msg) noexcept
      : code{code}, retval{0}, msg{msg} {}

  /**
   * @brief Construct a new Status object
   *
   * It defines the constructor to define a custom code and description
   *
   * @param code code of the error
   * @param retval return value in case of integer
   * @param msg description
   */
  Status(const int code, const int retval, const std::string &msg) noexcept
      : code{code}, retval{retval}, msg{msg} {}
};
}  // namespace cynq
