#include "PerformanceMixerModel.h"

PerformanceMixerModel::PerformanceMixerModel(LayersListModel *layers,
                                             QObject *parent) : QObject(parent),
  _layers(layers) {}

LayersListModel *PerformanceMixerModel::layers() const {
  return _layers;
}

std::optional<std::pair<int, char>>
parse_pmx_input_name(std::string_view name) {
  constexpr std::string_view prefix = "pmx-input-";
  constexpr std::string_view suffix = "-input";

  if (!name.starts_with(prefix)) return std::nullopt;
  if (!name.ends_with(suffix)) return std::nullopt;

  constexpr std::size_t pos_digits = prefix.size();
  constexpr std::size_t pos_dash = pos_digits + 2;
  constexpr std::size_t pos_char = pos_dash + 1;
  constexpr std::size_t pos_suffix = pos_char + 1;

  if (name.size() != pos_suffix + suffix.size()) return std::nullopt;

  const char d0 = name[pos_digits + 0];
  const char d1 = name[pos_digits + 1];
  if (!std::isdigit(static_cast<unsigned char>(d0)) || !std::isdigit(
    static_cast<unsigned char>(d1))) {
    return std::nullopt;
  }
  const int number = (d0 - '0') * 10 + (d1 - '0');

  if (name[pos_dash] != '-') return std::nullopt;

  const char layer = name[pos_char];
  if (layer < 'a' || layer > 'z') return std::nullopt;

  return std::pair{number, layer};
}

std::optional<std::pair<int, char>>
parse_pmx_group_name(std::string_view name) {
  constexpr std::string_view prefix = "pmx-group-";
  constexpr std::string_view suffix = "-input";

  if (!name.starts_with(prefix)) return std::nullopt;
  if (!name.ends_with(suffix)) return std::nullopt;

  constexpr std::size_t pos_digits = prefix.size();
  constexpr std::size_t pos_dash = pos_digits + 2;
  constexpr std::size_t pos_char = pos_dash + 1;
  constexpr std::size_t pos_suffix = pos_char + 1;

  if (name.size() != pos_suffix + suffix.size()) return std::nullopt;

  const char d0 = name[pos_digits + 0];
  const char d1 = name[pos_digits + 1];
  if (!std::isdigit(static_cast<unsigned char>(d0)) || !std::isdigit(
    static_cast<unsigned char>(d1))) {
    return std::nullopt;
  }
  const int number = (d0 - '0') * 10 + (d1 - '0');

  if (name[pos_dash] != '-') return std::nullopt;

  const char layer = name[pos_char];
  if (layer < 'a' || layer > 'z') return std::nullopt;

  return std::pair{number, layer};
}

void PerformanceMixerModel::update_target_object_for_node_name(
  std::string node_name, std::optional<std::string> target_object) {
  auto input_channel_id_and_layer_char = parse_pmx_input_name(node_name);
  if (input_channel_id_and_layer_char.has_value()) {
    const int input_channel_id = input_channel_id_and_layer_char.value().first;
    const char layer_char = input_channel_id_and_layer_char.value().second;
    const int layer_index = layer_char - 'a';
    _layers->update_input_channel_target_object(layer_index, input_channel_id,
                                                target_object);
  }
}

void PerformanceMixerModel::initial_channel_update(wireplumber_node &node) {
  if (node.name.has_value()) {
    auto name = std::string_view(node.name.value());
    if (name.starts_with("pmx-input-") && name.ends_with("-input")) {
      auto input_channel_id_and_layer_char = parse_pmx_input_name(name);
      const int input_channel_id = input_channel_id_and_layer_char.value().
        first;
      const char layer_char = input_channel_id_and_layer_char.value().second;
      const int layer_index = layer_char - 'a';
      _layers->
        initial_input_channel_update(layer_index, input_channel_id, node);
    } else if (name.starts_with("pmx-group-") && name.ends_with("-input")) {
      auto group_channel_id_and_layer_char = parse_pmx_group_name(name);
      const int group_channel_id = group_channel_id_and_layer_char.value().
        first;
      const char layer_char = group_channel_id_and_layer_char.value().second;
      const int layer_index = layer_char - 'a';
      _layers->
        initial_group_channel_update(layer_index, group_channel_id, node);
    } else if (name.starts_with("pmx-layer-") && name.ends_with("-input")) {
      auto layer_char = name[10];
      const int layer_index = layer_char - 'a';
      _layers->initial_layer_channel_update(layer_index, node);
    }
  }
}
