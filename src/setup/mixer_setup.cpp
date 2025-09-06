#include "mixer_setup.h"

#include <format>

mixer_setup create_default_mixer_setup(bool create_both_layers,
                                       size_t number_of_input_channels,
                                       size_t number_of_groups_channels) {
  mixer_setup setup{};
  setup.layers.emplace_back();

  auto &left = setup.layers.front();
  left.layer_channel.name = "Left Layer";
  left.layer_channel.pipewire_node_name_prefix = "pmx-layer-a";

  for (size_t i = 0; i < number_of_input_channels; i++) {
    left.input_channels.emplace_back();
    left.input_channels.back().channel_index = i;
    left.input_channels.back().name = std::format("Channel {}", i + 1);
    left.input_channels.back().connect_to_group_index = i / 4;
    left.input_channels.back().pipewire_node_name_perfix = std::format(
      "pmx-input-{:02}-a", i);
  }

  const std::array<std::string, 4> group_names = {
    "Drums", "Bass", "Melody", "Atmos"
  };

  for (size_t i = 0; i < number_of_groups_channels; i++) {
    left.group_channels.emplace_back();
    left.group_channels.back().channel_index = i;
    left.group_channels.back().name = group_names[i];
    left.group_channels.back().pipewire_node_name_perfix = std::format(
      "pmx-group-{:02}-a", i);
  }

  if (create_both_layers) {
    setup.layers.emplace_back();

    auto &right = setup.layers.back();
    right.layer_channel.name = "Right Layer";
    right.layer_channel.pipewire_node_name_prefix = "pmx-layer-b";

    for (size_t i = 0; i < number_of_input_channels; i++) {
      right.input_channels.emplace_back();
      right.input_channels.back().channel_index = i + 16;
      right.input_channels.back().name = std::format("Channel {}", i + 1);
      right.input_channels.back().connect_to_group_index = i / 4;
      right.input_channels.back().pipewire_node_name_perfix = std::format(
        "pmx-input-{:02}-b", i);
    }

    for (size_t i = 0; i < number_of_groups_channels; i++) {
      right.group_channels.emplace_back();
      right.group_channels.back().channel_index = i;
      right.group_channels.back().name = group_names[i];
      right.group_channels.back().pipewire_node_name_perfix = std::format(
        "pmx-group-{:02}-b", i);
    }
  }

  setup.output_channels.emplace_back();
  setup.output_channels.back().channel_index = 0;
  setup.output_channels.back().name = "Main";
  setup.output_channels.back().pipewire_node_name_perfix = "pmx-output-00";

  return setup;
}
