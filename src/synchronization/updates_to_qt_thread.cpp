#include "updates_to_qt_thread.h"

#include <qobjectdefs.h>
#include <spa/pod/iter.h>

void updates_to_qt_thread(
  std::vector<std::shared_ptr<updates_to_qt_thread_data>> feedback_updates,
  std::shared_ptr<feedback_ctrl> feedback_ctrl) {
  while (feedback_ctrl->running) {
    queue_message message;
    for (auto &thread_data : feedback_updates) {
      while (thread_data->value.read(message)) {
        QMetaObject::invokeMethod(thread_data->channel_strips_model,
                                  [thread_data, message]() mutable {
                                    auto pod = static_cast<spa_pod*>(
                                      spa_pod_from_data(
                                        message.buffer.data(), MAX_VALUE_SIZE,
                                        0, message.size));
                                    thread_data->channel_strips_model->
                                                 apply_update(
                                                   thread_data->channel_index,
                                                   pod);
                                  }, Qt::QueuedConnection);
      };
    }
    feedback_ctrl->has_value.wait(false);
    feedback_ctrl->has_value = false;
  }
}
