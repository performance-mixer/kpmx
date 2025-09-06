#pragma once

#include <memory>

#include "synchronization/lock_free_value.h"

struct pipewire_control_data;
struct feedback_ctrl;
struct updates_to_qt_thread_data;

int pipewire_monitor_thread(int argc, char *argv[],
                            std::vector<std::shared_ptr<
                              updates_to_qt_thread_data>> feedback_updates);

int pipewire_control_thread(int argc, char *argv[],
                            const std::shared_ptr<pipewire_control_data>
                            pipewire_control_data);

int pipewire_feedback_thread(int argc, char *argv[],
                             std::vector<std::shared_ptr<
                               updates_to_qt_thread_data>> feedback_updates,
                             std::shared_ptr<feedback_ctrl> feedback_ctrl);
