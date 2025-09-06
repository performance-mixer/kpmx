#pragma once

#include <optional>
#include <string>
#include <vector>

struct input_channel_setup {
  std::string name;
  std::optional<size_t> connect_to_group_index;
  size_t channel_index;
  std::string pipewire_node_name_perfix;
};

using channel_strips = std::vector<input_channel_setup>;

struct group_channel_setup {
  std::string name;
  size_t channel_index;
  std::string pipewire_node_name_perfix;
};

using group_channel_strips = std::vector<group_channel_setup>;

struct layer_channel_setup {
  std::string name;
  std::string pipewire_node_name_prefix;
};

struct layer_setup {
  channel_strips input_channels;
  group_channel_strips group_channels;
  layer_channel_setup layer_channel;
};

using layer_setups = std::vector<layer_setup>;

struct output_channel_setup {
  std::string name;
  size_t channel_index;
  std::string pipewire_node_name_perfix;
};

using output_channels = std::vector<output_channel_setup>;

struct mixer_setup {
  layer_setups layers;
  ::output_channels output_channels;
};

mixer_setup create_default_mixer_setup(bool create_both_layers,
                                       size_t number_of_input_channels,
                                       size_t number_of_groups_channels);
