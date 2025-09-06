#include "LayersListModel.h"

#include <utility>

LayersListModel::LayersListModel(QObject *parent) : QAbstractListModel(
  parent) {}

int LayersListModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) return 0;
  return static_cast<int>(_layers.size());
}

QVariant LayersListModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return {};

  if (role == NameRole) {
    return _layers.at(index.row())->name();
  }

  if (role == LayerRole) {
    return QVariant::fromValue<LayerModel*>(_layers.at(index.row()));
  }

  return {};
}

QHash<int, QByteArray> LayersListModel::roleNames() const {
  return {{NameRole, "name"}, {LayerRole, "mixerLayer"},};
}

void LayersListModel::addLayer(QString name,
                               ChannelStripListModel *input_channels,
                               ChannelStripListModel *group_channels,
                               ChannelStripListModel *layer_channel) {
  const int newRow = static_cast<int>(_layers.size());
  beginInsertRows(QModelIndex(), newRow, newRow);
  auto *layer = new LayerModel(this);
  layer->setName(std::move(name));
  layer->setInputChannels(input_channels);
  layer->setGroupChannels(group_channels);
  layer->setLayerChannels(layer_channel);
  _layers.push_back(layer);
  endInsertRows();
}

void LayersListModel::update_input_channel_target_object(int layer_index,
  int channel_index, const std::optional<std::string> &target_object) {
  auto layer = _layers.at(layer_index);
  layer->input_channels()->update_target_object(channel_index, target_object);
}

void LayersListModel::initial_input_channel_update(int layer_index,
                                                   int channel_index,
                                                   wireplumber_node &node) {
  const auto layer = _layers.at(layer_index);
  layer->input_channels()->update_from_wireplumber_node(channel_index, node);
}

void LayersListModel::initial_group_channel_update(int layer_index,
                                                   int group_index,
                                                   wireplumber_node &node) {
  const auto layer = _layers.at(layer_index);
  layer->group_channels()->update_from_wireplumber_node(group_index, node);
}

void LayersListModel::initial_layer_channel_update(int layer_index,
                                                   wireplumber_node &node) {
  const auto layer = _layers.at(layer_index);
  layer->layer_channels()->update_from_wireplumber_node(0, node);
}
