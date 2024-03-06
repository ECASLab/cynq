/*
 * See LICENSE for more information about licensing
 *
 * Copyright 2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <xrt.h>
#include <xrt/xrt_bo.h>
#include <xrt/xrt_device.h>
#pragma GCC diagnostic pop

#include <cynq/accelerator.hpp>
#include <cynq/alveo/hardware.hpp>
#include <cynq/datamover.hpp>
#include <cynq/enums.hpp>
#include <cynq/hardware.hpp>
#include <cynq/status.hpp>
#include <memory>
#include <stdexcept>
#include <string>

namespace cynq {
Alveo::Alveo(const std::string & /*bitstream_file*/,
             const std::string &xclbin_file)
    : parameters_{std::make_shared<AlveoParameters>()} {
  AlveoParameters *params =
      dynamic_cast<AlveoParameters *>(this->parameters_.get());
  /* For the Alveo, there is only a single device. It is possible to
     load only a xclbin. */
  Status st{};

  /* Initial check: we want to make sure that both parameters are OK */
  if (xclbin_file.empty()) {
    throw std::runtime_error("Cannot work with an empty XCLBIN file");
  }
  params->xclbin_file_ = xclbin_file;

  st = this->Reset();
  if (st.code != Status::OK) {
    std::string msg = "Error while configuring the buses: ";
    msg += st.msg;
    throw std::runtime_error(msg);
  }
}

Status Alveo::LoadXclBin(const std::string &xclbin_file, const int device_idx) {
  AlveoParameters *params =
      dynamic_cast<AlveoParameters *>(this->parameters_.get());
  if (!params) {
    return Status{Status::INCOMPATIBLE_PARAMETER,
                  "Hardware params incompatible"};
  }

  params->device_ = xrt::device(device_idx);
  params->uuid_ = params->device_.load_xclbin(xclbin_file);
  params->xclbin_ = xrt::xclbin(xclbin_file);

  return Status{};
}

Status Alveo::Reset() {
  AlveoParameters *params =
      dynamic_cast<AlveoParameters *>(this->parameters_.get());
  /* Configure the buses accordingly to the default design
  TODO: add device idx */
  return LoadXclBin(params->xclbin_file_);
}

std::shared_ptr<IDataMover> Alveo::GetDataMover(const uint64_t address) {
  return IDataMover::Create(IDataMover::XRT, address, this->parameters_);
}

std::shared_ptr<IAccelerator> Alveo::GetAccelerator(
    const uint64_t /* address */) {
  return nullptr;
}

std::shared_ptr<IAccelerator> Alveo::GetAccelerator(
    const std::string &kernelname) {
  return IAccelerator::Create(IAccelerator::XRT, kernelname, parameters_);
}

Alveo::~Alveo() {}

}  // namespace cynq
