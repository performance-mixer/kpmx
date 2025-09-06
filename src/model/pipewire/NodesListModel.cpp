#include "NodesListModel.h"

#include <qproperty.h>
#include <pipewire/node.h>

#include "wireplumber/wireplumber_object_manager_thread.h"

NodesListModel::NodesListModel(QObject *parent) : QAbstractListModel(parent) {}

int NodesListModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) return 0;
  return static_cast<int>(_nodes.size());
}

QVariant NodesListModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return {};

  const auto i = index.row();

  if (role == NameRole) {
    return QString::fromStdString(_nodes.at(i).name.value_or("unknown"));
  }

  if (role == IdRole) {
    if (_nodes.at(i).id.has_value()) {
      return _nodes.at(i).id.value();
    }

    return {};
  }

  if (role == ObjectSerialRole) {
    if (_nodes.at(i).object_serial.has_value()) {
      return _nodes.at(i).object_serial.value();
    }

    return {};
  }

  if (role == NameDescription) {
    return QString::fromStdString(_nodes.at(i).description.value_or(""));
  }

  if (role == DeviceId) {
    if (_nodes.at(i).device_id.has_value()) {
      return _nodes.at(i).device_id.value();
    } else {
      return {};
    }
  }

  if (role == TargetObject) {
    if (_nodes.at(i).target_object.has_value()) {
      return QString::fromStdString(_nodes.at(i).target_object.value());
    } else {
      return {};
    }
  }

  if (role == MediaRole) {
    if (_nodes.at(i).media_role.has_value()) {
      return QString::fromStdString(_nodes.at(i).media_role.value());
    } else {
      return {};
    }
  }

  if (role == MediaClass) {
    if (_nodes.at(i).media_class.has_value()) {
      return QString::fromStdString(_nodes.at(i).media_class.value());
    } else {
      return {};
    }
  }

  if (role == MediaCategory) {
    if (_nodes.at(i).media_category.has_value()) {
      return QString::fromStdString(_nodes.at(i).media_category.value());
    } else {
      return {};
    }
  }

  if (role == MediaType) {
    if (_nodes.at(i).media_type.has_value()) {
      return QString::fromStdString(_nodes.at(i).media_type.value());
    } else {
      return {};
    }
  }

  if (role == LinkGroup) {
    if (_nodes.at(i).link_group.has_value()) {
      return QString::fromStdString(_nodes.at(i).link_group.value());
    } else {
      return {};
    }
  }

  return {};
}

QHash<int, QByteArray> NodesListModel::roleNames() const {
  return {
    {NameRole, "name"}, {IdRole, "id"}, {ObjectSerialRole, "objectSerial"},
    {NameDescription, "description"}, {DeviceId, "deviceId"},
    {TargetObject, "targetObject"}, {MediaRole, "mediaRole"},
    {MediaClass, "mediaClass"}, {MediaCategory, "mediaCategory"},
    {MediaType, "mediaType"}, {LinkGroup, "linkGroup"}
  };
}

void NodesListModel::addNode(wireplumber_node node) {
  const int newRow = static_cast<int>(_nodes.size());
  beginInsertRows(QModelIndex(), newRow, newRow);
  _nodes.push_back(node);
  endInsertRows();
}

void NodesListModel::deleteNodeByObjectSerial(int object_serial) {
  auto to_delete = std::find_if(_nodes.begin(), _nodes.end(),
                                [object_serial](const auto &node) {
                                  return node.object_serial.has_value() && node.
                                    object_serial.value() == object_serial;
                                });

  if (std::end(_nodes) != to_delete) {
    const int row = std::distance(_nodes.begin(), to_delete);
    beginRemoveRows(QModelIndex(), row, row);
    _nodes.erase(to_delete);
    endRemoveRows();
  }
}

void NodesListModel::updateNodeTargetObject(int object_id,
                                            const std::optional<std::string> &
                                            target_object) {
  auto to_update = std::find_if(_nodes.begin(), _nodes.end(),
                                [object_id](const auto &node) {
                                  return node.id.has_value() && node.id.value()
                                    == object_id;
                                });

  if (std::end(_nodes) != to_update) {
    const int row = std::distance(_nodes.begin(), to_update);
    to_update->target_object = target_object;
    Q_EMIT dataChanged(index(row, 0), index(row, 0),
                       QVector<int>{TargetObject});
  }
}

std::optional<wireplumber_node>
NodesListModel::node_by_object_id(int object_id) {
  auto item = std::find_if(_nodes.begin(), _nodes.end(),
                           [object_id](const auto &node) {
                             return node.id.has_value() && node.id.value() ==
                               object_id;
                           });
  if (item != std::end(_nodes)) {
    return *item;
  }

  return std::nullopt;
}
