#pragma once
#include <atomic>
#include <vector>

#include "synchronization/lock_free_queue.h"

struct channel_data {
  lock_free_queue queue = nullptr;
  std::string control_node_name;
};

struct pipewire_control_data {
  std::atomic<bool> running;
  std::atomic<bool> has_value;
  std::vector<channel_data> channels;
};
