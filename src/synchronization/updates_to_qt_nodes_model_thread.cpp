#include "updates_to_qt_nodes_model_thread.h"

#include <memory>
#include <qobjectdefs.h>

#include "wireplumber/wireplumber_node.h"

void updates_to_qt_nodes_model_thread(
  updates_to_qt_nodes_model_data *thread_ctrl) {
  auto update_model = [&thread_ctrl](
    const wireplumber_object_manager_queue_message &message) -> void {
    if (message.operation == wireplumber_object_manager_operation::Insert) {
      auto node = std::get<wireplumber_node>(message.object.value());
      QMetaObject::invokeMethod(thread_ctrl->nodes_model,
                                [thread_ctrl, message, node]() mutable {
                                  thread_ctrl->nodes_model->addNode(node);
                                }, Qt::QueuedConnection);
      QMetaObject::invokeMethod(thread_ctrl->performance_mixer_model,
                                [thread_ctrl, node]() mutable {
                                  thread_ctrl->performance_mixer_model->
                                               initial_channel_update(node);
                                });
    } else if (message.operation ==
      wireplumber_object_manager_operation::Delete) {
      auto node = std::get<wireplumber_node>(message.object.value());
      QMetaObject::invokeMethod(thread_ctrl->nodes_model,
                                [thread_ctrl, node]() mutable {
                                  thread_ctrl->nodes_model->
                                               deleteNodeByObjectSerial(
                                                 node.object_serial.value());
                                }, Qt::QueuedConnection);
    } else if (message.operation ==
      wireplumber_object_manager_operation::MetadataUpdate) {
      auto update = std::get<wireplumber_metadata_update>(
        message.object.value());
      QMetaObject::invokeMethod(thread_ctrl->nodes_model,
                                [thread_ctrl, update]() mutable {
                                  if (update.key == "target_object") {
                                    thread_ctrl->nodes_model->
                                                 updateNodeTargetObject(
                                                   update.id, update.value);
                                    auto node = thread_ctrl->nodes_model->
                                      node_by_object_id(update.id);
                                    if (node.has_value()) {
                                      thread_ctrl->performance_mixer_model->
                                                   update_target_object_for_node_name(
                                                     node.value().name.value(),
                                                     update.value);
                                    }
                                  }
                                });
    }
  };

  while (thread_ctrl->running) {
    thread_ctrl->queue->consume_all([&update_model](const auto &update) {
      update_model(update);
    });

    thread_ctrl->has_value.wait(false);
    thread_ctrl->has_value = false;
  }
}
