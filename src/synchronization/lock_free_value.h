#pragma once

#include <atomic>

#include <boost/lockfree/spsc_value.hpp>

#define MAX_VALUE_SIZE 4096

struct queue_message {
  std::array<uint8_t, MAX_VALUE_SIZE> buffer;
  size_t size;
};

using lock_free_value = boost::lockfree::spsc_value<queue_message>;
