#pragma once

#include <qabstractitemmodel.h>

#include "ChannelStripModel.h"

class ChannelStripListModel : public QAbstractListModel {
public:
  enum Roles { NameRole = Qt::UserRole + 1, ChannelStripRole };

  explicit ChannelStripListModel(QObject *parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant
  data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  void addChannelStrip(QString name, lock_free_queue &queue,
                       ::metadata_updates_queue metadata_updates_queue,
                       std::atomic<bool> &has_value);

  void apply_update(size_t index, spa_pod *control_pod);

  void update_target_object(size_t index,
                            std::optional<std::string> target_object);

  void update_from_wireplumber_node(int channel_index, wireplumber_node &node);

private:
  std::vector<ChannelStripModel*> _channelStrips;
};
