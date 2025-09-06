#pragma once

#include <QObject>

#include "ChannelStripListModel.h"

class LayerModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(ChannelStripListModel* inputChannels READ input_channels WRITE setInputChannels NOTIFY inputChannelsChanged)
  Q_PROPERTY(ChannelStripListModel* groupChannels READ group_channels WRITE setGroupChannels NOTIFY groupChannelsChanged)
  Q_PROPERTY(ChannelStripListModel* layerChannels READ layer_channels WRITE setLayerChannels NOTIFY layerChannelsChanged)
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

public:
  explicit LayerModel(QObject* parent = nullptr);

  [[nodiscard]] QString name() const;
  void setName(QString name);

  [[nodiscard]] ChannelStripListModel* input_channels() const;
  void setInputChannels(ChannelStripListModel* input_channels);

  [[nodiscard]] ChannelStripListModel* group_channels() const;
  void setGroupChannels(ChannelStripListModel* group_channels);

  [[nodiscard]] ChannelStripListModel* layer_channels() const;
  void setLayerChannels(ChannelStripListModel* layer_channel);

private:
  ChannelStripListModel *_input_channels = nullptr;
  ChannelStripListModel *_group_channels = nullptr;
  ChannelStripListModel *_layer_channels = nullptr;
  QString _name;

  Q_SIGNAL void inputChannelsChanged();
  Q_SIGNAL void groupChannelsChanged();
  Q_SIGNAL void layerChannelsChanged();
  Q_SIGNAL void nameChanged();
};

Q_DECLARE_METATYPE(LayerModel*)
