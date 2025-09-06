#include <array>

#include <spa/control/control.h>
#include <spa/param/format.h>
#include <spa/param/props.h>
#include <spa/param/tag-utils.h>
#include <spa/param/audio/raw-json.h>
#include <spa/pod/dynamic.h>
#include <spa/utils/ringbuffer.h>
#include <spa/filter-graph/filter-graph.h>
#include <pipewire/pipewire.h>

#include "pipewire_thread.h"

#include <string>
#include <format>
#include <memory>

#include "synchronization/updates_to_qt_thread.h"

struct stream_info_feedback {
  struct pw_stream *stream{};
  struct pw_properties *props{};
  struct spa_hook listener{};
  std::shared_ptr<updates_to_qt_thread_data> feedback_updates = nullptr;
  std::shared_ptr<::feedback_ctrl> feedback_ctrl = nullptr;
};

struct impl {
  struct pw_main_loop *loop{};
  struct pw_core *core{};
  struct std::list<stream_info_feedback> feedback_streams;
};

inline void stream_destroy(void *data) {
  auto *stream = static_cast<stream_info_feedback*>(data);
  spa_hook_remove(&stream->listener);
  stream->stream = nullptr;
}

inline static void stream_state_changed([[maybe_unused]] void *data,
                                        [[maybe_unused]] enum pw_stream_state
                                        old, enum pw_stream_state state,
                                        [[maybe_unused]] const char *error) {
  //auto *stream = static_cast<stream_info_feedback*>(data);

  switch (state) {
  case PW_STREAM_STATE_PAUSED:
    break;
  case PW_STREAM_STATE_UNCONNECTED:
    break;
  case PW_STREAM_STATE_ERROR:
    break;
  case PW_STREAM_STATE_STREAMING:
    break;
  default:
    break;
  }
}

void feedback_process(void *data) {
  auto stream_info = static_cast<stream_info_feedback*>(data);

  struct pw_buffer *feedback = pw_stream_dequeue_buffer(stream_info->stream);
  if (feedback == nullptr) {
    fprintf(stderr, "Failed to dequeue feedback buffer\n");
    return;
  }

  if (feedback->buffer->n_datas > 0) {
    const auto pod = static_cast<struct spa_pod*>(spa_pod_from_data(
      feedback->buffer->datas[0].data, feedback->buffer->datas[0].maxsize,
      feedback->buffer->datas[0].chunk->offset,
      feedback->buffer->datas[0].chunk->size));

    if (spa_pod_is_sequence(pod)) {
      struct spa_pod_control *pod_control;
      struct spa_pod_control *pod_control_end = nullptr;
      SPA_POD_SEQUENCE_FOREACH((struct spa_pod_sequence *)pod, pod_control) {
        if (pod_control->type == SPA_CONTROL_Properties) {
          pod_control_end = pod_control;
        }
      }

      if (pod_control_end != nullptr) {
        queue_message message{};
        auto size = SPA_POD_SIZE(&pod_control_end->value);
        std::memcpy(message.buffer.data(), &pod_control_end->value, size);
        message.size = size;
        stream_info->feedback_updates->value.write(message);
        stream_info->feedback_ctrl->has_value = true;
        stream_info->feedback_ctrl->has_value.notify_all();
      }
    }
  }

  pw_stream_queue_buffer(stream_info->stream, feedback);
}

static const struct pw_stream_events feedback_stream_events = {
  PW_VERSION_STREAM_EVENTS, stream_destroy, stream_state_changed, nullptr,
  nullptr, nullptr, nullptr, nullptr, feedback_process, nullptr, nullptr,
  nullptr
};

inline void signal_exit(void *data, [[maybe_unused]] int signal_number) {
  auto impl = static_cast<struct impl*>(data);
  pw_main_loop_quit(impl->loop);
}

int pipewire_feedback_thread(int argc, char *argv[],
                             std::vector<std::shared_ptr<
                               updates_to_qt_thread_data>> feedback_updates,
                             std::shared_ptr<feedback_ctrl> feedback_ctrl) {
  impl impl;
  pw_init(&argc, &argv);

  impl.loop = pw_main_loop_new(nullptr);
  auto *context = pw_context_new(pw_main_loop_get_loop(impl.loop), nullptr, 0);
  impl.core = pw_context_connect(context, nullptr, 0);

  uint8_t index = 0;

  index = 0;
  for (auto &stream_info : feedback_updates) {
    auto &feedback_stream = impl.feedback_streams.emplace_back();
    feedback_stream.feedback_ctrl = feedback_ctrl;
    feedback_stream.feedback_updates = feedback_updates[index];
    feedback_stream.props = pw_properties_new(nullptr, nullptr);
    pw_properties_set(feedback_stream.props, PW_KEY_MEDIA_CLASS,
                      "Stream/Input/Control");
    pw_properties_set(feedback_stream.props, PW_KEY_MEDIA_ROLE, "Control");
    pw_properties_set(feedback_stream.props, PW_KEY_NODE_LINK_GROUP,
                      "kpmx-feedback");
    pw_properties_set(feedback_stream.props, PW_KEY_NODE_AUTOCONNECT, "true");
    pw_properties_set(feedback_stream.props, PW_KEY_MEDIA_TYPE, "Control");
    pw_properties_set(feedback_stream.props, PW_KEY_MEDIA_CATEGORY, "Capture");

    auto node_name = std::format("kpmx-{:02}-feedback",
                                 static_cast<int>(index));
    pw_properties_set(feedback_stream.props, PW_KEY_NODE_NAME,
                      node_name.c_str());

    pw_properties_set(feedback_stream.props, PW_KEY_TARGET_OBJECT,
                      stream_info->feedback_node_name.c_str());

    feedback_stream.stream = pw_stream_new(impl.core, node_name.c_str(),
                                           feedback_stream.props);
    if (feedback_stream.stream == nullptr) {
      fprintf(stderr, "Failed to create stream\n");
      return 1;
    }

    pw_stream_add_listener(feedback_stream.stream, &feedback_stream.listener,
                           &feedback_stream_events, &feedback_stream);

    uint8_t buffer[256];
    spa_pod_builder builder{};
    spa_pod_builder_init(&builder, buffer, sizeof(buffer));
    const struct spa_pod *param[1];

    spa_pod_frame frame{};
    spa_pod_builder_push_object(&builder, &frame, SPA_TYPE_OBJECT_Format,
                                SPA_PARAM_EnumFormat);
    spa_pod_builder_add(&builder, SPA_FORMAT_mediaType,
                        SPA_POD_Id(SPA_MEDIA_TYPE_application),
                        SPA_FORMAT_mediaSubtype,
                        SPA_POD_Id(SPA_MEDIA_SUBTYPE_control), 0);
    param[0] = static_cast<const struct spa_pod*>(spa_pod_builder_pop(
      &builder, &frame));

    pw_stream_connect(feedback_stream.stream, PW_DIRECTION_INPUT, PW_ID_ANY,
                      static_cast<enum pw_stream_flags>(
                        PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS
                        | PW_STREAM_FLAG_RT_PROCESS), param, 1);

    index++;
  }

  struct pw_loop *pw_loop = pw_main_loop_get_loop(impl.loop);
  pw_loop_add_signal(pw_loop, SIGINT, signal_exit, impl.loop);
  pw_loop_add_signal(pw_loop, SIGTERM, signal_exit, impl.loop);
  pw_main_loop_run(impl.loop);

  for (auto &feedback_stream : impl.feedback_streams) {
    pw_stream_disconnect(feedback_stream.stream);
    pw_stream_destroy(feedback_stream.stream);
    pw_properties_free(feedback_stream.props);
  }

  if (impl.core) {
    pw_core_disconnect(impl.core);
  }

  pw_deinit();

  return 0;
}
