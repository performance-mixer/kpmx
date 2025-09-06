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

struct stream_info {
  struct pw_stream *stream;
  struct pw_properties *props;
  struct spa_hook listener;
  uint8_t buffer[256];
};

struct impl {
  struct pw_main_loop *loop{};
  struct pw_core *core{};
  struct std::list<stream_info> monitor_streams{};
};

inline void stream_destroy(void *data) {
  auto *stream = static_cast<stream_info*>(data);
  spa_hook_remove(&stream->listener);
  stream->stream = nullptr;
}

inline static void stream_state_changed(void *data,
                                        [[maybe_unused]] enum pw_stream_state
                                        old, enum pw_stream_state state,
                                        [[maybe_unused]] const char *error) {
  [[maybe_unused]] auto *stream = static_cast<stream_info*>(data);

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

inline void monitor_process(void *data) {
  auto si = static_cast<stream_info*>(data);
  struct pw_buffer *b = pw_stream_dequeue_buffer(si->stream);
  if (!b) return;
  pw_stream_queue_buffer(si->stream, b);
}

static const struct pw_stream_events monitor_stream_events = {
  PW_VERSION_STREAM_EVENTS, stream_destroy, stream_state_changed, nullptr,
  nullptr, nullptr, nullptr, nullptr, monitor_process, nullptr, nullptr, nullptr
};

inline void signal_exit(void *data, [[maybe_unused]] int signal_number) {
  auto impl = static_cast<struct impl*>(data);
  pw_main_loop_quit(impl->loop);
}

int pipewire_monitor_thread(int argc, char *argv[],
                            std::vector<std::shared_ptr<
                              updates_to_qt_thread_data>> feedback_updates) {
  impl impl;
  pw_init(&argc, &argv);

  impl.loop = pw_main_loop_new(nullptr);
  auto *context = pw_context_new(pw_main_loop_get_loop(impl.loop), nullptr, 0);
  impl.core = pw_context_connect(context, nullptr, 0);

  uint8_t index = 0;
  for (auto &stream_setup : feedback_updates) {
    auto &monitor_stream = impl.monitor_streams.emplace_back();
    monitor_stream.props = pw_properties_new(nullptr, nullptr);
    pw_properties_set(monitor_stream.props, PW_KEY_MEDIA_CLASS,
                      "Stream/Input/Audio");
    pw_properties_set(monitor_stream.props, PW_KEY_MEDIA_ROLE, "Capture");
    pw_properties_set(monitor_stream.props, PW_KEY_NODE_LINK_GROUP, "kpmx");
    pw_properties_set(monitor_stream.props, PW_KEY_NODE_AUTOCONNECT, "false");
    pw_properties_set(monitor_stream.props, PW_KEY_MEDIA_TYPE, "Audio");
    pw_properties_set(monitor_stream.props, PW_KEY_MEDIA_CATEGORY, "Capture");

    auto node_name = std::format("kpmx-{:02}-monitor", static_cast<int>(index));
    pw_properties_set(monitor_stream.props, PW_KEY_NODE_NAME,
                      node_name.c_str());

    pw_properties_set(monitor_stream.props, PW_KEY_TARGET_OBJECT,
                      stream_setup->monitor_node_name.c_str());

    monitor_stream.stream = pw_stream_new(impl.core, node_name.c_str(),
                                          monitor_stream.props);
    if (monitor_stream.stream == nullptr) {
      fprintf(stderr, "Failed to create stream\n");
      return 1;
    }

    pw_stream_add_listener(monitor_stream.stream, &monitor_stream.listener,
                           &monitor_stream_events, &monitor_stream);

    spa_pod_builder builder{};
    spa_pod_builder_init(&builder, monitor_stream.buffer,
                         sizeof(monitor_stream.buffer));
    const struct spa_pod *param[1];

    spa_pod_frame frame{};
    spa_pod_builder_push_object(&builder, &frame, SPA_TYPE_OBJECT_Format,
                                SPA_PARAM_EnumFormat);
    spa_pod_builder_add(&builder, SPA_FORMAT_mediaType,
                        SPA_POD_Id(SPA_MEDIA_TYPE_audio),
                        SPA_FORMAT_mediaSubtype,
                        SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw),
                        SPA_FORMAT_AUDIO_format,
                        SPA_POD_Id(SPA_AUDIO_FORMAT_F32P),
                        SPA_FORMAT_AUDIO_rate, SPA_POD_Int(48000),
                        SPA_FORMAT_AUDIO_channels, SPA_POD_Int(2), 0);
    param[0] = static_cast<const struct spa_pod*>(spa_pod_builder_pop(
      &builder, &frame));

    pw_stream_connect(monitor_stream.stream, PW_DIRECTION_INPUT, PW_ID_ANY,
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

  for (auto &monitor_stream : impl.monitor_streams) {
    pw_stream_disconnect(monitor_stream.stream);
    pw_stream_destroy(monitor_stream.stream);
    pw_properties_free(monitor_stream.props);
  }

  if (impl.core) {
    pw_core_disconnect(impl.core);
  }

  pw_deinit();

  return 0;
}
