#pragma once

#include "ATen/cuda/ATenCUDAGeneral.h"
#include "ATen/cuda/CUDAContext.h"
#include "ATen/cuda/CUDAStream.h"
#include "ATen/cuda/CUDAGuard.h"
#include "ATen/cuda/Exceptions.h"
#include "c10/util/Exception.h"

#include "cuda_runtime_api.h"

#include <cstdint>
#include <utility>

namespace at { namespace cuda {

/*
* CUDAEvents are movable not copyable wrappers around CUDA's events.
*
* CUDAEvents are constructed lazily when recorded on streams. The events
* have a device, and this device is acquired from the first recording stream.
* Later streams that record to the event must share this device, but streams
* on any device can wait on the event.
*/
struct AT_CUDA_API CUDAEvent {
  // Constants
  static constexpr unsigned int DEFAULT_FLAGS = cudaEventDisableTiming;

  // Constructors
  CUDAEvent(unsigned int flags = DEFAULT_FLAGS)
  : flags_{flags} { }

  // Note: event destruction done on creating device to avoid creating a
  // CUDA context on other devices.
  ~CUDAEvent() {
    try {
      if (is_created_) {
<<<<<<< dest:   0504088ae558 - ezyang: Add c10::Stream, make at::cuda::CUDASt...
        at::DeviceGuard device_guard{static_cast<int16_t>(device_index_)};
||||||| base
        at::DeviceGuard device_guard{(int)device_index_};
=======
        at::cuda::CUDAGuard device_guard(static_cast<int>(device_index_));
>>>>>>> source: aa097875d8ce - ezyang: Delete DeviceGuard(int64_t) constructor.
        cudaEventDestroy(event_);
      }
    } catch (...) { /* No throw */ }
  }

  CUDAEvent(const CUDAEvent&) = delete;
  CUDAEvent& operator=(const CUDAEvent&) = delete;

  CUDAEvent(CUDAEvent&& other) { moveHelper(std::move(other)); }
  CUDAEvent& operator=(CUDAEvent&& other) {
    moveHelper(std::move(other));
    return *this;
  }

  operator cudaEvent_t() const { return event(); }

  // Less than operator (to allow use in sets)
  friend bool operator<(const CUDAEvent& left, const CUDAEvent& right) {
    return left.event_ < right.event_;
  }

  bool isCreated() const { return is_created_; }
  int64_t device() const { return device_index_; }
  cudaEvent_t event() const { return event_; }

  bool happened() const {
    return (was_recorded_ && cudaEventQuery(event_) == cudaSuccess);
  }

  void record() { record(getCurrentCUDAStream()); }

  void recordOnce(const CUDAStream& stream) {
    if (!was_recorded_) record(stream);
  }

  void record(const CUDAStream& stream) {
    if (is_created_) {
      AT_ASSERT(device_index_ == stream.device_index());
    } else {
      create(stream.device_index());
    }

    AT_CUDA_CHECK(cudaEventRecord(event_, stream));
    was_recorded_ = true;
  }

  void block (const CUDAStream& stream) {
    if (is_created_) {
      AT_CUDA_CHECK(cudaStreamWaitEvent(stream, event_, 0));
    }
  }


private:
  unsigned int flags_ = DEFAULT_FLAGS;
  bool is_created_ = false;
  bool was_recorded_ = false;
  int64_t device_index_ = -1;
  cudaEvent_t event_;

  void moveHelper(CUDAEvent&& other) {
    std::swap(flags_, other.flags_);
    std::swap(is_created_, other.is_created_);
    std::swap(was_recorded_, other.was_recorded_);
    std::swap(device_index_, other.device_index_);
    std::swap(event_, other.event_);
  }

  void create(const int64_t device) {
    at::cuda::CUDAGuard device_index_guard(static_cast<int16_t>(device));
    AT_CUDA_CHECK(cudaEventCreateWithFlags(&event_, flags_));

    is_created_ = true;
    device_index_ = device;
  }
};

} // namespace cuda
} // namespace at
