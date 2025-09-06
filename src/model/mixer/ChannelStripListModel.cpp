#include "ChannelStripListModel.h"

ChannelStripListModel::ChannelStripListModel(QObject *parent)
  : QAbstractListModel(parent) {}

int ChannelStripListModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) return 0;
  return static_cast<int>(_channelStrips.size());
}

QVariant ChannelStripListModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return {};

  if (role == NameRole) {
    return _channelStrips.at(index.row())->name();
  }

  if (role == ChannelStripRole) {
    return QVariant::fromValue<ChannelStripModel*>(
      _channelStrips.at(index.row()));
  }

  return {};
}

QHash<int, QByteArray> ChannelStripListModel::roleNames() const {
  return {{NameRole, "name"}, {ChannelStripRole, "channelStrip"},};
}

void ChannelStripListModel::addChannelStrip(QString name,
                                            lock_free_queue &queue,
                                            ::metadata_updates_queue
                                            metadata_updates_queue,
                                            std::atomic<bool> &has_value) {
  const int newRow = static_cast<int>(_channelStrips.size());
  beginInsertRows(QModelIndex(), newRow, newRow);
  auto *channelStrip = new ChannelStripModel(queue, metadata_updates_queue,
                                             has_value, this);
  channelStrip->setName(name);
  _channelStrips.push_back(channelStrip);
  endInsertRows();
}

void ChannelStripListModel::apply_update(size_t index, spa_pod *control_pod) {
  if (_channelStrips.size() > index) {
    _channelStrips.at(index)->apply(control_pod);
  }
}

void ChannelStripListModel::update_target_object(
  size_t index, std::optional<std::string> target_object) {
  if (_channelStrips.size() > index) {
    _channelStrips.at(index)->setTargetObject(target_object);
  }
}

void ChannelStripListModel::update_from_wireplumber_node(int channel_index,
  wireplumber_node &node) {
  if (_channelStrips.size() > channel_index) {
    _channelStrips.at(channel_index)->setObjectId(node.id);
    _channelStrips.at(channel_index)->setTargetObject(node.target_object);
  }
}
