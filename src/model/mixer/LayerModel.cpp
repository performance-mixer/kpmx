#include "LayerModel.h"

#include <utility>

LayerModel::LayerModel(QObject *parent) : QObject(parent) {}

ChannelStripListModel * LayerModel::input_channels() const {
  return _input_channels;
}

void LayerModel::setInputChannels(ChannelStripListModel *input_channels) {
  _input_channels = input_channels;
  Q_EMIT inputChannelsChanged();
}

ChannelStripListModel * LayerModel::group_channels() const {
  return _group_channels;
}

void LayerModel::setGroupChannels(ChannelStripListModel *group_channels) {
  _group_channels = group_channels;
  Q_EMIT groupChannelsChanged();
}

ChannelStripListModel * LayerModel::layer_channels() const {
  return _layer_channels;
}

void LayerModel::setLayerChannels(ChannelStripListModel *layer_channels) {
  _layer_channels = layer_channels;
  Q_EMIT layerChannelsChanged();
}

QString LayerModel::name() const {
  return _name;
}

void LayerModel::setName(QString name) {
  _name = std::move(name);
  Q_EMIT nameChanged();
}
