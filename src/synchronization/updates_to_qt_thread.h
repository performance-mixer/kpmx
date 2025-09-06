#pragma once

#include <memory>

#include "lock_free_value.h"
#include "model/mixer/ChannelStripListModel.h"

struct updates_to_qt_thread_data {
  lock_free_value value;
  ChannelStripListModel *channel_strips_model;
  size_t channel_index;
  std::string feedback_node_name;
  std::string control_node_name;
  std::string monitor_node_name;
};

struct feedback_ctrl {
  std::atomic<bool> has_value;
  std::atomic<bool> running;
};

void updates_to_qt_thread(
  std::vector<std::shared_ptr<updates_to_qt_thread_data>> feedback_updates,
  std::shared_ptr<feedback_ctrl> feedback_ctrl);
