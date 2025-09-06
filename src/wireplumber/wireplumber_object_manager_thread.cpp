#include "wireplumber_object_manager_thread.h"

#include <glib.h>
#include <glib-unix.h>
#include <iostream>
#include <ostream>
#include <thread>

#include <pipewire/keys.h>

#include <wp/core.h>
#include <wp/wp.h>

struct impl {
  std::atomic<bool> &has_value;
  GOptionContext *context = nullptr;
  GMainLoop *loop = nullptr;
  GMainContext *main_context = nullptr;
  WpCore *core = nullptr;
  WpObjectManager *om = nullptr;
  WpMetadata *metadata = nullptr;
  boost::lockfree::spsc_queue<wireplumber_object_manager_queue_message> *queue =
    nullptr;
};

void fill_node(WpPipewireObject *g_object, wireplumber_node &node) {
  const char *props = nullptr;
  if ((props = wp_pipewire_object_get_property(g_object, PW_KEY_OBJECT_ID)) !=
    nullptr) {
    node.id = std::stoi(props);
  }

  if ((props = wp_pipewire_object_get_property(g_object, PW_KEY_OBJECT_SERIAL))
    != nullptr) {
    node.object_serial = std::stoi(props);
  }

  if ((props = wp_pipewire_object_get_property(g_object, PW_KEY_NODE_NAME)) !=
    nullptr) {
    node.name = std::string(props);
  }

  if ((props = wp_pipewire_object_get_property(g_object,
                                               PW_KEY_NODE_DESCRIPTION)) !=
    nullptr) {
    node.description = std::string(props);
  }

  if ((props = wp_pipewire_object_get_property(g_object, PW_KEY_DEVICE_ID)) !=
    nullptr) {
    node.device_id = std::stoi(props);
  }

  if ((props = wp_pipewire_object_get_property(g_object, PW_KEY_TARGET_OBJECT))
    != nullptr) {
    node.target_object = std::string(props);
  }

  if ((props = wp_pipewire_object_get_property(g_object, PW_KEY_MEDIA_ROLE)) !=
    nullptr) {
    node.media_role = std::string(props);
  }

  if ((props = wp_pipewire_object_get_property(g_object, PW_KEY_MEDIA_CLASS)) !=
    nullptr) {
    node.media_class = std::string(props);
  }

  if ((props = wp_pipewire_object_get_property(g_object, PW_KEY_MEDIA_CATEGORY))
    != nullptr) {
    node.media_category = std::string(props);
  }

  if ((props = wp_pipewire_object_get_property(g_object, PW_KEY_MEDIA_TYPE)) !=
    nullptr) {
    node.media_type = std::string(props);
  }

  if ((props =
      wp_pipewire_object_get_property(g_object, PW_KEY_NODE_LINK_GROUP)) !=
    nullptr) {
    node.link_group = std::string(props);
  }
}

static void metadata_changed_callback([[maybe_unused]] WpMetadata *metadata,
                                      guint32 subject, const gchar *key_char,
                                      [[maybe_unused]] const gchar *type_char,
                                      const gchar *value_char,
                                      gpointer user_data) {
  auto *impl = static_cast<::impl*>(user_data);

  std::optional<std::string> value = std::nullopt;
  if (value_char) {
    value = std::string(value_char);
  }

  wireplumber_object_manager_queue_message message{
    .object = wireplumber_metadata_update{
      .id = static_cast<int>(subject), .key = std::string(key_char),
      .value = value,
    },
    .operation = wireplumber_object_manager_operation::MetadataUpdate,
  };

  impl->queue->push(message);
  impl->has_value = true;
  impl->has_value.notify_all();
}

auto node_added_callback = +[ ](
  [[maybe_unused]] WpObjectManager *object_manager, gpointer object,
  gpointer user_data) {
    impl *impl = static_cast<::impl*>(user_data);

    if (WP_IS_METADATA(object)) {
      const auto metadata = static_cast<WpMetadata*>(object);
      GValue global_properties_gvalue{0};
      g_object_get_property(G_OBJECT(metadata), "global-properties",
                            &global_properties_gvalue);

      if (global_properties_gvalue.data->v_pointer) {
        const auto global_properties = static_cast<WpProperties*>(
          global_properties_gvalue.data->v_pointer);
        auto name_c = wp_properties_get(global_properties, "metadata.name");
        if (name_c) {
          if (const std::string name(name_c); name == "default") {
            g_signal_connect(metadata, "changed",
                             G_CALLBACK(+metadata_changed_callback), impl);

            impl->metadata = metadata;
          }
        }
      }
    } else if (WP_IS_NODE(object)) {
      const auto g_object = static_cast<WpPipewireObject*>(object);
      wireplumber_node node{};
      fill_node(g_object, node);

      const wireplumber_object_manager_queue_message message{
        .object = node,
        .operation = wireplumber_object_manager_operation::Insert
      };

      impl->queue->push(message);
      impl->has_value = true;
      impl->has_value.notify_all();
    }
  };

auto node_removed_callback = +[ ](
  [[maybe_unused]] WpObjectManager *object_manager, gpointer object,
  gpointer user_data) {
    impl *impl = static_cast<::impl*>(user_data);

    const auto g_object = static_cast<WpPipewireObject*>(object);
    wireplumber_node node{};
    fill_node(g_object, node);

    const wireplumber_object_manager_queue_message message{
      .object = node, .operation = wireplumber_object_manager_operation::Delete
    };

    impl->queue->push(message);
    impl->has_value = true;
    impl->has_value.notify_all();
  };

struct my_data {
  WpMetadata *metadata;
  std::string key;
  std::optional<std::string> value;
  int subject;
};

void wireplumber_metadata_update_thread(metadata_updates_queue queue,
                                        std::atomic<bool> &running,
                                        std::atomic<bool> &has_value,
                                        impl *impl) {
  while (running) {
    queue->consume_all([impl](const auto &update) {
      if (impl->metadata != nullptr) {
        auto data = new my_data{
          .metadata = impl->metadata, .key = update.key, .value = update.value,
          .subject = update.id,
        };

        g_idle_add_once(+[](gpointer user_data) {
          auto *data = static_cast<my_data*>(user_data);
          wp_metadata_set(data->metadata, data->subject, data->key.c_str(),
                          "Spa:String",
                          data->value.has_value()
                            ? data->value.value().c_str()
                            : nullptr);
          delete data;
        }, data);
      }
    });
    has_value.wait(false);
    has_value = false;
  }
}

void wireplumber_object_manager_thread(wireplumber_object_manager_data *data) {
  impl impl{.has_value = data->has_value,};
  impl.queue = data->queue;
  wp_init(WP_INIT_ALL);
  impl.context = g_option_context_new("metadata-manager");
  impl.main_context = g_main_context_new();
  impl.loop = g_main_loop_new(impl.main_context, false);
  impl.core = wp_core_new(impl.main_context, nullptr, nullptr);
  if (wp_core_connect(impl.core)) {
    impl.om = wp_object_manager_new();

    wp_object_manager_add_interest(impl.om, WP_TYPE_NODE, nullptr);
    wp_object_manager_request_object_features(impl.om, WP_TYPE_NODE,
                                              WP_OBJECT_FEATURES_ALL);

    wp_object_manager_add_interest(impl.om, WP_TYPE_METADATA, nullptr);
    wp_object_manager_request_object_features(impl.om, WP_TYPE_METADATA,
                                              WP_OBJECT_FEATURES_ALL);

    wp_core_install_object_manager(impl.core, impl.om);

    g_signal_connect(impl.om, "object-added", G_CALLBACK(+node_added_callback),
                     &impl);
    g_signal_connect(impl.om, "object-removed",
                     G_CALLBACK(+node_removed_callback), &impl);

    auto signal_handler = []([[maybe_unused]] int signum, gpointer user_data) {
      auto loop = static_cast<GMainLoop*>(user_data);
      g_main_loop_quit(loop);
    };

    g_unix_signal_add(SIGINT, G_SOURCE_FUNC(+signal_handler), impl.loop);
    g_unix_signal_add(SIGTERM, G_SOURCE_FUNC(+signal_handler), impl.loop);

    std::atomic<bool> metadata_thread_running{true};
    std::thread metadata_update_thread(wireplumber_metadata_update_thread,
                                       data->metadata_updates_queue,
                                       std::ref(metadata_thread_running),
                                       std::ref(data->has_value), &impl);

    g_main_loop_run(impl.loop);

    g_object_unref(impl.om);
    wp_core_disconnect(impl.core);
    g_object_unref(impl.core);
    g_main_loop_unref(impl.loop);
    g_main_context_unref(impl.main_context);
    g_option_context_free(impl.context);
  }
}
