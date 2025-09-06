#pragma once
#include <atomic>
#include <memory>
#include <boost/lockfree/spsc_queue.hpp>

#include "model/mixer/PerformanceMixerModel.h"
#include "model/pipewire/NodesListModel.h"
#include "wireplumber/wireplumber_node.h"
#include "wireplumber/wireplumber_object_manager_thread.h"

struct updates_to_qt_nodes_model_data {
  std::atomic<bool> &has_value;
  std::atomic<bool> running;
  boost::lockfree::spsc_queue<wireplumber_object_manager_queue_message> *queue;
  NodesListModel *nodes_model;
  PerformanceMixerModel *performance_mixer_model;
};

void updates_to_qt_nodes_model_thread(
  updates_to_qt_nodes_model_data *thread_ctrl);
