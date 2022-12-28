#pragma once

#include <future>

namespace handbag::start_stop {

struct IStartable {
  virtual ~IStartable() = default;

  virtual std::future<void> start();
};

struct IStoppable {
  virtual ~IStoppable() = default;

  virtual std::future<void> stop();
};

struct IStartableStoppable : public IStartable, public IStoppable {};

}  // namespace handbag::start_stop
