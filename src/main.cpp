#include <KIconTheme>
#include <KLocalizedString>
#include <QApplication>
#include <QQuickStyle>
#include <QtQml>
#include <csignal>

#include "pipewire/pipewire_thread.h"

#include "model/mixer/ChannelStripListModel.h"
#include "model/mixer/ChannelStripModel.h"
#include "model/mixer/LayerModel.h"
#include "model/mixer/LayersListModel.h"
#include "model/mixer/PerformanceMixerModel.h"
#include "model/pipewire/NodesListModel.h"
#include "pipewire/pipewire_control_data.h"
#include "setup/mixer_setup.h"
#include "synchronization/updates_to_qt_nodes_model_thread.h"
#include "synchronization/updates_to_qt_thread.h"
#include "wireplumber/wireplumber_object_manager_thread.h"

int main(int argc, char *argv[]) {
  KIconTheme::initTheme();
  QApplication app(argc, argv);
  KLocalizedString::setApplicationDomain("tutorial");
  QApplication::setOrganizationName(QStringLiteral("KDE"));
  QApplication::setOrganizationDomain(QStringLiteral("performance-mixer.org"));
  QApplication::setApplicationName(QStringLiteral("Performance Mixer"));
  QApplication::setDesktopFileName(
    QStringLiteral("org.performance_mixer.kpmx"));

  QApplication::setStyle(QStringLiteral("breeze"));
  if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
  }

  QQmlApplicationEngine engine;

  qmlRegisterUncreatableType<ChannelStripModel>("org.performance_mixer.kpmx", 1,
                                                0, "ChannelStripModel",
                                                QStringLiteral(
                                                  "Provided by C++"));
  qmlRegisterUncreatableType<ChannelStripListModel>(
    "org.performance_mixer.kpmx", 1, 0, "ChannelStripListModel",
    QStringLiteral("Provided by C++"));
  qmlRegisterUncreatableType<LayerModel>("org.performance_mixer.kpmx", 1, 0,
                                         "LayerModel",
                                         QStringLiteral("Provided by C++"));
  qmlRegisterUncreatableType<LayersListModel>("org.performance_mixer.kpmx", 1,
                                              0, "LayersListModel",
                                              QStringLiteral(
                                                "Provided by C++"));
  qmlRegisterUncreatableType<PerformanceMixerModel>(
    "org.performance_mixer.kpmx", 1, 0, "PerformanceMixerModel",
    QStringLiteral("Provided by C++"));

  const auto setup = create_default_mixer_setup(false, 8, 2);

  std::vector<std::shared_ptr<updates_to_qt_thread_data>> pipewire_qt_sync_info;
  auto pipewire_control_data = std::make_shared<::pipewire_control_data>();
  pipewire_control_data->has_value = false;
  pipewire_control_data->running = true;

  wireplumber_object_manager_data wireplumber_object_manager_data{};
  wireplumber_object_manager_data.queue = new boost::lockfree::spsc_queue<
    wireplumber_object_manager_queue_message>(1024);
  wireplumber_object_manager_data.metadata_updates_queue = new
    boost::lockfree::spsc_queue<wireplumber_metadata_update>(1024);
  wireplumber_object_manager_data.has_value = false;

  auto layer_list_model = new LayersListModel();
  size_t layer_index = 0;
  for (const auto &layer_setup : setup.layers) {
    auto input_channels_model = new ChannelStripListModel();
    size_t channel_index = 0;
    for (auto &input_channel_setup : layer_setup.input_channels) {
      auto &control_data = pipewire_control_data->channels.emplace_back();
      control_data.control_node_name = input_channel_setup.
        pipewire_node_name_perfix + "-control";
      control_data.queue = new boost::lockfree::spsc_queue<
        control_queue_message>(1024);
      input_channels_model->addChannelStrip(
        QString::fromStdString(input_channel_setup.name), control_data.queue,
        wireplumber_object_manager_data.metadata_updates_queue,
        std::ref(wireplumber_object_manager_data.has_value));
      pipewire_qt_sync_info.push_back(
        std::make_shared<updates_to_qt_thread_data>());
      pipewire_qt_sync_info.back()->channel_strips_model = input_channels_model;
      pipewire_qt_sync_info.back()->channel_index = channel_index;
      pipewire_qt_sync_info.back()->feedback_node_name = input_channel_setup.
        pipewire_node_name_perfix + "-feedback";
      pipewire_qt_sync_info.back()->control_node_name = input_channel_setup.
        pipewire_node_name_perfix + "-control";
      pipewire_qt_sync_info.back()->monitor_node_name = input_channel_setup.
        pipewire_node_name_perfix + "-monitor";
      ++channel_index;
    }

    auto group_channels_model = new ChannelStripListModel();
    channel_index = 0;
    for (auto &group_channel_setup : layer_setup.group_channels) {
      auto &control_data = pipewire_control_data->channels.emplace_back();
      control_data.control_node_name = group_channel_setup.
        pipewire_node_name_perfix + "-control";
      control_data.queue = new boost::lockfree::spsc_queue<
        control_queue_message>(1024);
      group_channels_model->addChannelStrip(
        QString::fromStdString(group_channel_setup.name), control_data.queue,
        wireplumber_object_manager_data.metadata_updates_queue,
        std::ref(wireplumber_object_manager_data.has_value));
      pipewire_qt_sync_info.push_back(
        std::make_shared<updates_to_qt_thread_data>());
      pipewire_qt_sync_info.back()->channel_strips_model = group_channels_model;
      pipewire_qt_sync_info.back()->channel_index = channel_index;
      pipewire_qt_sync_info.back()->feedback_node_name = group_channel_setup.
        pipewire_node_name_perfix + "-feedback";
      pipewire_qt_sync_info.back()->control_node_name = group_channel_setup.
        pipewire_node_name_perfix + "-control";
      pipewire_qt_sync_info.back()->monitor_node_name = group_channel_setup.
        pipewire_node_name_perfix + "-monitor";
      ++channel_index;
    }

    auto layer_channels_model = new ChannelStripListModel();

    auto &control_data = pipewire_control_data->channels.emplace_back();
    control_data.control_node_name = layer_setup.layer_channel.
                                                 pipewire_node_name_prefix +
      "-control";
    control_data.queue = new boost::lockfree::spsc_queue<
      control_queue_message>(1024);

    auto layer_name = std::format("Layer {}", layer_index);
    layer_channels_model->addChannelStrip(QString::fromStdString(layer_name),
                                          control_data.queue,
                                          wireplumber_object_manager_data.
                                          metadata_updates_queue,
                                          std::ref(
                                            wireplumber_object_manager_data.
                                            has_value));

    layer_list_model->addLayer(QString::fromStdString(layer_name),
                               input_channels_model, group_channels_model,
                               layer_channels_model);
    pipewire_qt_sync_info.push_back(
      std::make_shared<updates_to_qt_thread_data>());
    pipewire_qt_sync_info.back()->channel_strips_model = layer_channels_model;
    pipewire_qt_sync_info.back()->channel_index = layer_index;
    pipewire_qt_sync_info.back()->feedback_node_name = layer_setup.layer_channel
      .pipewire_node_name_prefix + "-feedback";
    pipewire_qt_sync_info.back()->control_node_name = layer_setup.layer_channel.
      pipewire_node_name_prefix + "-control";
    pipewire_qt_sync_info.back()->monitor_node_name = layer_setup.layer_channel.
      pipewire_node_name_prefix + "-monitor";
    ++layer_index;
  }

  static PerformanceMixerModel performance_mixer_model(layer_list_model);
  qmlRegisterSingletonInstance("org.performance_mixer.kpmx", 1, 0, "Mixer",
                               &performance_mixer_model);

  static NodesListModel nodes_list_model;
  qmlRegisterSingletonInstance("org.performance_mixer.kpmx", 1, 0, "Nodes",
                               &nodes_list_model);

  engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
  engine.loadFromModule("org.performance_mixer.kpmx", "Main");

  if (engine.rootObjects().isEmpty()) {
    return -1;
  }

  std::thread wireplumber_object_manager_thread(
    ::wireplumber_object_manager_thread, &wireplumber_object_manager_data);

  updates_to_qt_nodes_model_data updates_to_qt_nodes_model_data{
    .has_value = wireplumber_object_manager_data.has_value, .running = true,
    .queue = wireplumber_object_manager_data.queue,
    .nodes_model = &nodes_list_model,
    .performance_mixer_model = &performance_mixer_model,
  };
  std::thread updates_to_qt_node_model_thread(updates_to_qt_nodes_model_thread,
                                              &updates_to_qt_nodes_model_data);

  std::thread pipewire_monitor(pipewire_monitor_thread, argc, argv,
                               pipewire_qt_sync_info);
  std::thread pipewire_control(pipewire_control_thread, argc, argv,
                               pipewire_control_data);

  auto feedback_thread_ctrl = std::make_shared<feedback_ctrl>();
  feedback_thread_ctrl->running = true;

  std::thread pipewire_feedback(pipewire_feedback_thread, argc, argv,
                                pipewire_qt_sync_info, feedback_thread_ctrl);

  std::thread feedback_updates_thread(updates_to_qt_thread,
                                      pipewire_qt_sync_info,
                                      feedback_thread_ctrl);

  auto q_return = QApplication::exec();

  pthread_kill(pipewire_monitor.native_handle(), SIGINT);
  pthread_kill(pipewire_control.native_handle(), SIGINT);
  pthread_kill(pipewire_feedback.native_handle(), SIGINT);
  pipewire_monitor.join();
  pipewire_feedback.join();
  pipewire_control.join();

  feedback_thread_ctrl->running = false;
  feedback_thread_ctrl->has_value = true;
  feedback_thread_ctrl->has_value.notify_all();
  feedback_updates_thread.join();

  pthread_kill(wireplumber_object_manager_thread.native_handle(), SIGINT);
  wireplumber_object_manager_thread.join();

  updates_to_qt_nodes_model_data.running = false;
  updates_to_qt_nodes_model_data.has_value = true;
  updates_to_qt_nodes_model_data.has_value.notify_all();
  updates_to_qt_node_model_thread.join();
  return q_return;
}
