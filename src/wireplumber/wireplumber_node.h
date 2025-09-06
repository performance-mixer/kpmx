#pragma once

#include <variant>
#include <optional>
#include <string>

struct wireplumber_node {
  std::optional<int> id;
  std::optional<int> object_serial;
  std::optional<std::string> name;
  std::optional<std::string> description;
  std::optional<int> device_id;
  std::optional<std::string> target_object;
  std::optional<std::string> media_role;
  std::optional<std::string> media_class;
  std::optional<std::string> media_category;
  std::optional<std::string> media_type;
  std::optional<std::string> link_group;
};

struct wireplumber_metadata_update {
  int id;
  std::string key;
  std::optional<std::string> value;
};

using wireplumber_objects = std::variant<
  wireplumber_node, wireplumber_metadata_update>;

enum class wireplumber_object_manager_operation {
  Insert, Delete, MetadataUpdate
};

struct wireplumber_object_manager_queue_message {
  std::optional<wireplumber_objects> object;
  wireplumber_object_manager_operation operation =
    wireplumber_object_manager_operation::Insert;
};
