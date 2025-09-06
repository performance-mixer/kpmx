#pragma once

#include <qabstractitemmodel.h>

#include "wireplumber/wireplumber_node.h"

class NodesListModel : public QAbstractListModel {
  Q_OBJECT public:
  enum Roles {
    NameRole = Qt::UserRole + 1,
    IdRole,
    ObjectSerialRole,
    NameDescription,
    DeviceId,
    TargetObject,
    MediaRole,
    MediaClass,
    MediaCategory,
    MediaType,
    LinkGroup
  };

  explicit NodesListModel(QObject *parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant
  data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  void addNode(wireplumber_node node);
  void deleteNodeByObjectSerial(int object_serial);
  void updateNodeTargetObject(int object_id,
                              const std::optional<std::string> &target_object);

  std::optional<wireplumber_node> node_by_object_id(int object_id);

private:
  std::vector<wireplumber_node> _nodes;
};
