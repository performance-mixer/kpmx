#pragma once
#include "ChannelStripListModel.h"
#include "LayerModel.h"

class LayersListModel : public QAbstractListModel {
  Q_OBJECT public:
  enum Roles { NameRole = Qt::UserRole + 1, LayerRole };

  explicit LayersListModel(QObject *parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant
  data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  void addLayer(QString name, ChannelStripListModel *input_channels,
                ChannelStripListModel *group_channels,
                ChannelStripListModel *layer_channel);

  void update_input_channel_target_object(int layer_index, int channel_index,
                                          const std::optional<std::string> &
                                          target_object);

  void initial_input_channel_update(int layer_index, int channel_index,
                                    wireplumber_node &node);

  void initial_group_channel_update(int layer_index, int group_index,
                                    wireplumber_node &node);

  void initial_layer_channel_update(int layer_index, wireplumber_node &node);

private:
  std::vector<LayerModel*> _layers;
};
