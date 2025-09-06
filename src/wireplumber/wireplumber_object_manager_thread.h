#pragma once

#include <memory>
#include <boost/lockfree/spsc_queue.hpp>

#include "wireplumber_node.h"

using metadata_updates_queue = boost::lockfree::spsc_queue<wireplumber_metadata_update>*;

struct wireplumber_object_manager_data {
  boost::lockfree::spsc_queue<wireplumber_object_manager_queue_message> *queue;
  std::atomic<bool> has_value;
  ::metadata_updates_queue metadata_updates_queue;
  std::atomic<bool> metadata_has_value;
};

void wireplumber_object_manager_thread(
  wireplumber_object_manager_data *thread_ctrl);
