#pragma once

#include <QObject>

#include "LayersListModel.h"

class PerformanceMixerModel : public QObject {
  Q_OBJECT Q_PROPERTY(LayersListModel* layers READ layers CONSTANT)public:
  explicit PerformanceMixerModel(LayersListModel *layers,
                                 QObject *parent = nullptr);

  [[nodiscard]] LayersListModel *layers() const;

  void update_target_object_for_node_name(std::string node_name,
                                          std::optional<std::string> target_object);

  void initial_channel_update(wireplumber_node &node);

private:
  LayersListModel *_layers;
};
