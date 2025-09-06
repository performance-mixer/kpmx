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

#include "pipewire_control_data.h"
#include "synchronization/updates_to_qt_thread.h"

struct stream_info {
  struct pw_stream *stream;
  struct pw_properties *props;
  struct spa_hook listener;
  uint8_t buffer[256];
  lock_free_queue queue;
};

struct impl {
  struct pw_main_loop *loop{};
  struct pw_core *core{};
  struct std::list<stream_info> control_streams{};
};

inline void stream_destroy(void *data) {
  auto *stream = static_cast<stream_info*>(data);
  spa_hook_remove(&stream->listener);
  stream->stream = nullptr;
}

inline static void stream_state_changed(void *data, [[maybe_unused]] enum pw_stream_state old,
                                        enum pw_stream_state state, [[maybe_unused]] const char *error) {
  auto *stream = static_cast<stream_info*>(data);

  switch (state) {
  case PW_STREAM_STATE_PAUSED:
    pw_stream_flush(stream->stream, false);
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

void control_process(void *userdata) {
  auto user_data = static_cast<stream_info*>(userdata);

  auto control = pw_stream_dequeue_buffer(user_data->stream);
  if (control == nullptr) {
    fprintf(stderr, "Failed to dequeue control buffer\n");
    return;
  }

  struct spa_buffer *spa_buffer = control->buffer;
  if (!spa_buffer || spa_buffer->n_datas < 1) {
    fprintf(stderr, "no data planes\n");
    return;
  }

  struct spa_data *data = &spa_buffer->datas[0];
  if (!data->data || !data->chunk || data->maxsize == 0) {
    fprintf(stderr, "invalid control buffer\n");
    return;
  }

  spa_pod_builder b{};
  spa_pod_builder_init(&b, data->data, data->maxsize);

  spa_pod_frame seq_f; {}
  spa_pod_builder_push_sequence(&b, &seq_f, 0);

  if (user_data->queue->read_available() > 0) {
    spa_pod_frame frame{};
    spa_pod_builder_control(&b, 0, SPA_CONTROL_Properties);
    spa_pod_builder_push_object(&b, &frame, SPA_TYPE_OBJECT_Props,
                                SPA_PARAM_Props);
    spa_pod_builder_prop(&b, SPA_PROP_params, 0);

    spa_pod_frame struct_frame{};
    spa_pod_builder_push_struct(&b, &struct_frame);

    // todo - deduplicate !
    auto append_to_struct = [&](const auto &update) {
      spa_pod_builder_string(&b, update.parameter_name.c_str());

      if (std::holds_alternative<uint>(update.value)) {
        spa_pod_builder_int(&b, std::get<uint>(update.value));
      } else if (std::holds_alternative<double>(update.value)) {
        spa_pod_builder_double(&b, std::get<double>(update.value));
      } else if (std::holds_alternative<bool>(update.value)) {
        spa_pod_builder_bool(&b, std::get<bool>(update.value));
      } else if (std::holds_alternative<int>(update.value)) {
        spa_pod_builder_int(&b, std::get<int>(update.value));
      }
    };

    user_data->queue->consume_all([&append_to_struct](const auto &update) {
      append_to_struct(update);
    });

    spa_pod_builder_pop(&b, &struct_frame);
    spa_pod_builder_pop(&b, &frame);
  }

  const struct spa_sequence *spa_sequence = static_cast<const struct
    spa_sequence*>(spa_pod_builder_pop(&b, &seq_f));
  const uint32_t seq_size = SPA_POD_SIZE(spa_sequence);
  data->chunk->offset = 0;
  data->chunk->size = seq_size;
  data->chunk->stride = 0;
  pw_stream_queue_buffer(user_data->stream, control);
}

static const struct pw_stream_events control_stream_events = {
  PW_VERSION_STREAM_EVENTS, stream_destroy, stream_state_changed, nullptr,
  nullptr, nullptr, nullptr, nullptr, control_process, nullptr, nullptr, nullptr
};

inline void signal_exit(void *data, [[maybe_unused]] int signal_number) {
  auto impl = static_cast<struct impl*>(data);
  pw_main_loop_quit(impl->loop);
}

int pipewire_control_thread(int argc, char *argv[],
                            const std::shared_ptr<pipewire_control_data>
                            pipewire_control_data) {
  impl impl;
  pw_init(&argc, &argv);

  impl.loop = pw_main_loop_new(nullptr);
  auto *context = pw_context_new(pw_main_loop_get_loop(impl.loop), nullptr, 0);
  impl.core = pw_context_connect(context, nullptr, 0);

  uint8_t index = 0;
  index = 0;
  for (auto &stream_info : pipewire_control_data->channels) {
    auto &control_stream = impl.control_streams.emplace_back();
    control_stream.queue = stream_info.queue;
    control_stream.props = pw_properties_new(nullptr, nullptr);
    pw_properties_set(control_stream.props, PW_KEY_MEDIA_CLASS,
                      "Stream/Output/Control");
    pw_properties_set(control_stream.props, PW_KEY_MEDIA_ROLE, "Production");
    pw_properties_set(control_stream.props, PW_KEY_NODE_LINK_GROUP,
                      "kpmx-control");
    pw_properties_set(control_stream.props, PW_KEY_NODE_AUTOCONNECT, "true");
    pw_properties_set(control_stream.props, PW_KEY_MEDIA_TYPE, "Control");
    pw_properties_set(control_stream.props, PW_KEY_MEDIA_CATEGORY, "Playback");

    auto node_name = std::format("kpmx-{:02}-control", static_cast<int>(index));
    pw_properties_set(control_stream.props, PW_KEY_NODE_NAME,
                      node_name.c_str());

    pw_properties_set(control_stream.props, PW_KEY_TARGET_OBJECT,
                      stream_info.control_node_name.c_str());

    control_stream.stream = pw_stream_new(impl.core, node_name.c_str(),
                                          control_stream.props);
    if (control_stream.stream == nullptr) {
      fprintf(stderr, "Failed to create stream\n");
      return 1;
    }

    pw_stream_add_listener(control_stream.stream, &control_stream.listener,
                           &control_stream_events, &control_stream);

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

    pw_stream_connect(control_stream.stream, PW_DIRECTION_OUTPUT, PW_ID_ANY,
                      static_cast<enum pw_stream_flags>(
                        PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS
                        | PW_STREAM_FLAG_RT_PROCESS), param, 1);

    index++;
  }

  struct pw_loop *pw_loop = pw_main_loop_get_loop(impl.loop);
  [[maybe_unused]] struct spa_source *sigint = pw_loop_add_signal(
    pw_loop, SIGINT, signal_exit, impl.loop);

  [[maybe_unused]] struct spa_source *sigterm = pw_loop_add_signal(
    pw_loop, SIGTERM, signal_exit, impl.loop);

  pw_main_loop_run(impl.loop);

  for (auto &control_stream : impl.control_streams) {
    pw_stream_disconnect(control_stream.stream);
    pw_stream_destroy(control_stream.stream);
    pw_properties_free(control_stream.props);
  }

  if (impl.core) {
    pw_core_disconnect(impl.core);
  }

  pw_deinit();

  return 0;
}
