#pragma once

#include <variant>
#include <boost/lockfree/spsc_queue.hpp>

#include "lock_free_value.h"

using control_value = std::variant<uint, int, bool, double>;

enum class pipewire_type { Params };

struct control_queue_message {
  std::string parameter_name;
  control_value value;
  pipewire_type type = pipewire_type::Params;
};

using lock_free_queue = boost::lockfree::spsc_queue<control_queue_message>*;
