#include "ChannelStripModel.h"

#include <QVariant>
#include <set>
#include <unordered_set>
#include <spa/param/props.h>
#include <spa/pod/parser.h>
#include <spa/pod/iter.h>

QVariant ChannelStripModel::target_object() const {
  if (_target_object.has_value()) {
    return QVariant(QString::fromStdString(_target_object.value()));
  }

  return {};
}

void ChannelStripModel::setTargetObject(QVariant target_object) {
  if (target_object.isValid() && _object_id.has_value()) {
    auto new_value = target_object.toString().toStdString();
    if (new_value != _target_object) {
      _target_object = new_value;
      _metadata_updates_queue->push({
        .id = _object_id.value(), .key = "target.object", .value = new_value
      });
      _has_value = true;
      _has_value.notify_all();
      Q_EMIT targetObjectChanged();
    }
  } else if (_object_id.has_value()) {
    if (_target_object.has_value()) {
      _target_object = std::nullopt;
      _metadata_updates_queue->push({
        .id = _object_id.value(), .key = "target.object", .value = std::nullopt
      });
      _has_value = true;
      _has_value.notify_all();
      Q_EMIT targetObjectChanged();
    }
  }
}

void ChannelStripModel::setTargetObject(
  const std::optional<std::string> &target_object) {
  if (_target_object != target_object) {
    _target_object = target_object;
    Q_EMIT targetObjectChanged();
  }
}

ChannelStripModel::ChannelStripModel(const lock_free_queue &queue,
                                     ::metadata_updates_queue
                                     metadata_updates_queue,
                                     std::atomic<bool> &has_value,
                                     QObject *parent) : QObject(parent),
  _queue(queue), _metadata_updates_queue(metadata_updates_queue),
  _has_value(has_value) {}

uint ChannelStripModel::selected_loop() const {
  return _selected_loop;
}

// TODO
void ChannelStripModel::setSelectedLoop(const uint selected_loop) {
  if (_selected_loop != selected_loop) {
    _selected_loop = selected_loop;
    Q_EMIT selectedLoopChanged();
  }
}

uint ChannelStripModel::loop_length_in_bars() const {
  return _loop_length_in_bars;
}

void ChannelStripModel::setLoopLengthInBars(const uint loop_length_in_bars) {
  if (_loop_length_in_bars != loop_length_in_bars) {
    _loop_length_in_bars = loop_length_in_bars;
    control_queue_message message{
      .parameter_name = "Looper:bars", .value = loop_length_in_bars
    };
    _queue->push(message);
    Q_EMIT loopLengthInBarsChanged();
  }
}

bool ChannelStripModel::looper_playing() const {
  return _looper_playing;
}

void ChannelStripModel::setLooperPlaying(bool looper_playing) {
  if (_looper_playing != looper_playing) {
    _looper_playing = looper_playing;
    Q_EMIT looperPlayingChanged();
  }
}

bool ChannelStripModel::looper_recording() const {
  return _looper_recording;
}

void ChannelStripModel::setLooperRecording(bool looper_recording) {
  if (_looper_recording != looper_recording) {
    _looper_recording = looper_recording;
    Q_EMIT looperRecordingChanged();
  }
}

double ChannelStripModel::looper_mix() const {
  return _looper_mix;
}

void ChannelStripModel::setLooperMix(double looper_mix) {
  if (_looper_mix != looper_mix) {
    _looper_mix = looper_mix;
    control_queue_message message{
      .parameter_name = "Looper:mix", .value = looper_mix
    };
    _queue->push(message);
    Q_EMIT looperMixChanged();
  }
}

bool ChannelStripModel::saturator_bypass_enabled() const {
  return _saturator_bypass_enabled;
}

void ChannelStripModel::setSaturatorBypassEnabled(
  bool saturator_bypass_enabled) {
  if (_saturator_bypass_enabled != saturator_bypass_enabled) {
    _saturator_bypass_enabled = saturator_bypass_enabled;
    control_queue_message message{
      .parameter_name = "Saturator:bypass", .value = saturator_bypass_enabled
    };
    _queue->push(message);
    Q_EMIT saturatorBypassEnabledChanged();
  }
}

double ChannelStripModel::saturator_in_gain() const {
  return _saturator_in_gain;
}

void ChannelStripModel::setSaturatorInGain(double saturator_in_gain) {
  if (_saturator_in_gain != saturator_in_gain) {
    _saturator_in_gain = saturator_in_gain;
    control_queue_message message{
      .parameter_name = "Saturator:level_in", .value = saturator_in_gain
    };
    _queue->push(message);
    Q_EMIT saturatorInGainChanged();
  }
}

double ChannelStripModel::saturator_drive() const {
  return _saturator_drive;
}

void ChannelStripModel::setSaturatorDrive(double saturator_drive) {
  if (_saturator_drive != saturator_drive) {
    _saturator_drive = saturator_drive;
    control_queue_message message{
      .parameter_name = "Saturator:drive", .value = saturator_drive
    };
    _queue->push(message);
    Q_EMIT saturatorDriveChanged();
  }
}

double ChannelStripModel::saturator_blend() const {
  return _saturator_blend;
}

void ChannelStripModel::setSaturatorBlend(double saturator_blend) {
  if (_saturator_blend != saturator_blend) {
    _saturator_blend = saturator_blend;
    control_queue_message message{
      .parameter_name = "Saturator:blend", .value = saturator_blend
    };
    _queue->push(message);
    Q_EMIT saturatorBlendChanged();
  }
}

double ChannelStripModel::saturator_out_gain() const {
  return _saturator_out_gain;
}

void ChannelStripModel::setSaturatorOutGain(double saturator_out_gain) {
  if (_saturator_out_gain != saturator_out_gain) {
    _saturator_out_gain = saturator_out_gain;
    control_queue_message message{
      .parameter_name = "Saturator:level_out", .value = saturator_out_gain
    };
    _queue->push(message);
    Q_EMIT saturatorOutGainChanged();
  }
}

bool ChannelStripModel::compressor_bypass_enabled() const {
  return _compressor_bypass_enabled;
}

void ChannelStripModel::setCompressorBypassEnabled(
  bool compressor_bypass_enabled) {
  if (_compressor_bypass_enabled != compressor_bypass_enabled) {
    _compressor_bypass_enabled = compressor_bypass_enabled;
    control_queue_message message{
      .parameter_name = "Compressor:bypass", .value = compressor_bypass_enabled
    };
    _queue->push(message);
    Q_EMIT compressorBypassEnabledChanged();
  }
}

double ChannelStripModel::compressor_threshold() const {
  return _compressor_threshold;
}

void ChannelStripModel::setCompressorThreshold(double compressor_threshold) {
  if (_compressor_threshold != compressor_threshold) {
    _compressor_threshold = compressor_threshold;
    control_queue_message message{
      .parameter_name = "Compressor:threshold", .value = compressor_threshold
    };
    _queue->push(message);
    Q_EMIT compressorThresholdChanged();
  }
}

double ChannelStripModel::compressor_ratio() const {
  return _compressor_ratio;
}

void ChannelStripModel::setCompressorRatio(double compressor_ratio) {
  if (_compressor_ratio != compressor_ratio) {
    _compressor_ratio = compressor_ratio;
    control_queue_message message{
      .parameter_name = "Compressor:ratio", .value = compressor_ratio
    };
    _queue->push(message);
    Q_EMIT compressorRatioChanged();
  }
}

double ChannelStripModel::compressor_attack() const {
  return _compressor_attack;
}

void ChannelStripModel::setCompressorAttack(double compressor_attack) {
  if (_compressor_attack != compressor_attack) {
    _compressor_attack = compressor_attack;
    control_queue_message message{
      .parameter_name = "Compressor:attack", .value = compressor_attack
    };
    _queue->push(message);
    Q_EMIT compressorAttackChanged();
  }
}

double ChannelStripModel::compressor_release() const {
  return _compressor_release;
}

void ChannelStripModel::setCompressorRelease(double compressor_release) {
  if (_compressor_release != compressor_release) {
    _compressor_release = compressor_release;
    control_queue_message message{
      .parameter_name = "Compressor:release", .value = compressor_release
    };
    _queue->push(message);
    Q_EMIT compressorReleaseChanged();
  }
}

double ChannelStripModel::compressor_knee() const {
  return _compressor_knee;
}

void ChannelStripModel::setCompressorKnee(double compressor_knee) {
  if (_compressor_knee != compressor_knee) {
    _compressor_knee = compressor_knee;
    control_queue_message message{
      .parameter_name = "Compressor:knee", .value = compressor_knee
    };
    _queue->push(message);
    Q_EMIT compressorKneeChanged();
  }
}

double ChannelStripModel::compressor_mix() const {
  return _compressor_mix;
}

void ChannelStripModel::setCompressorMix(double compressor_mix) {
  if (_compressor_mix != compressor_mix) {
    _compressor_mix = compressor_mix;
    control_queue_message message{
      .parameter_name = "Compressor:mix", .value = compressor_mix
    };
    _queue->push(message);
    Q_EMIT compressorMixChanged();
  }
}

double ChannelStripModel::compressor_gain() const {
  return _compressor_gain;
}

void ChannelStripModel::setCompressorGain(double compressor_gain) {
  if (_compressor_gain != compressor_gain) {
    _compressor_gain = compressor_gain;
    control_queue_message message{
      .parameter_name = "Compressor:makeup", .value = compressor_gain
    };
    _queue->push(message);
    Q_EMIT compressorGainChanged();
  }
}

double ChannelStripModel::equalizer_high_gain() const {
  return _equalizer_high_gain;
}

void ChannelStripModel::setEqualizerHighGain(double equalizer_high_gain) {
  if (_equalizer_high_gain != equalizer_high_gain) {
    _equalizer_high_gain = equalizer_high_gain;
    control_queue_message message{
      .parameter_name = "Equalizer:high", .value = equalizer_high_gain
    };
    _queue->push(message);
    Q_EMIT equalizerHighGainChanged();
  }
}

double ChannelStripModel::equalizer_mid_gain() const {
  return _equalizer_mid_gain;
}

void ChannelStripModel::setEqualizerMidGain(double equalizer_mid_gain) {
  if (_equalizer_mid_gain != equalizer_mid_gain) {
    _equalizer_mid_gain = equalizer_mid_gain;
    control_queue_message message{
      .parameter_name = "Equalizer:mid", .value = equalizer_mid_gain
    };
    _queue->push(message);
    Q_EMIT equalizerMidGainChanged();
  }
}

double ChannelStripModel::equalizer_low_gain() const {
  return _equalizer_low_gain;
}

void ChannelStripModel::setEqualizerLowGain(double equalizer_low_gain) {
  if (_equalizer_low_gain != equalizer_low_gain) {
    _equalizer_low_gain = equalizer_low_gain;
    control_queue_message message{
      .parameter_name = "Equalizer:low", .value = equalizer_low_gain
    };
    _queue->push(message);
    Q_EMIT equalizerLowGainChanged();
  }
}

QString ChannelStripModel::name() const {
  return _name;
}

void ChannelStripModel::setName(const QString &name) {
  if (_name != name) {
    _name = name;
    Q_EMIT nameChanged();
  }
}

double ChannelStripModel::volume() const {
  return _volume;
}

void ChannelStripModel::setVolume(const double volume) {
  if (_volume != volume) {
    _volume = volume;
    control_queue_message message{
      .parameter_name = "Equalizer:master", .value = volume,
    };
    _queue->push(message);
    Q_EMIT volumeChanged();
  }
}

double ChannelStripModel::pan() const {
  return _pan;
}

void ChannelStripModel::setPan(double pan) {
  if (_pan != pan) {
    _pan = pan;
    Q_EMIT panChanged();
  }
}

QVariant ChannelStripModel::object_id() const {
  if (_object_id.has_value()) {
    return QVariant::fromValue(_object_id.value());
  }

  return {};
}

void ChannelStripModel::setObjectId(const QVariant &object_id) {
  if (object_id.isValid() && object_id.canConvert<int>()) {
    _object_id = object_id.toInt();
    Q_EMIT objectIdChanged();
  }
}

void ChannelStripModel::setObjectId(std::optional<int> object_id) {
  if (object_id != _object_id) {
    _object_id = object_id;
    Q_EMIT objectIdChanged();
  }
}

void ChannelStripModel::apply(spa_pod *control_pod) {
  const struct spa_pod_prop *property = nullptr;
  auto object = reinterpret_cast<spa_pod_object*>(control_pod);
  SPA_POD_OBJECT_FOREACH(object, property) {
    switch (property->key) {
    case SPA_PROP_params:
      parse_spa_prop_params(&property->value);
      break;
    default:
      break;
    }
  }
}

using param_variant = std::variant<double, bool, int32_t>;
using param = std::tuple<std::string, param_variant>;

void ChannelStripModel::parse_spa_prop_params(const spa_pod *params_pod) {
  std::vector<param> params;
  struct spa_pod_parser parser{};
  struct spa_pod_frame frame{};
  spa_pod_parser_pod(&parser, params_pod);
  if (spa_pod_parser_push_struct(&parser, &frame) < 0) return;

  while (true) {
    const char *name = nullptr;
    float float_value = 0;
    double double_value = 0;
    bool bool_value = false;
    int32_t int_value = 0;

    if (spa_pod_parser_get_string(&parser, &name) > 0) {
      break;
    }

    if (name == nullptr) break;

    if (spa_pod_parser_get_float(&parser, &float_value) >= 0) {
      params.push_back(std::make_tuple(name, float_value));
    } else if (spa_pod_parser_get_double(&parser, &double_value) >= 0) {
      params.push_back(std::make_tuple(name, double_value));
    } else if (spa_pod_parser_get_bool(&parser, &bool_value) >= 0) {
      params.push_back(std::make_tuple(name, bool_value));
    } else if (spa_pod_parser_get_int(&parser, &int_value) >= 0) {
      params.push_back(std::make_tuple(name, int_value));
    } else {
      struct spa_pod *pod;
      spa_pod_parser_get_pod(&parser, &pod);
    }
  }

  auto cmp = [](const std::function<void(ChannelStripModel *)> &a,
                const std::function<void(ChannelStripModel *)> &b) {
    return &a < &b;
  };

  std::set<std::function<void(ChannelStripModel *)>, decltype(cmp)>
    signals_to_emit;
  for (auto &param : params) {
    auto &name = std::get<0>(param);
    auto &value = std::get<1>(param);
    if (name == "Looper:loop1") {
      int new_value{};
      if (std::holds_alternative<double>(value)) {
        new_value = static_cast<int>(std::get<double>(value));
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = std::get<int32_t>(value);
      }

      if (new_value == 1 && _selected_loop != 1) {
        _selected_loop = 1;
        signals_to_emit.insert(&ChannelStripModel::selectedLoopChanged);
      }
    } else if (name == "Looper:loop2") {
      int new_value{};
      if (std::holds_alternative<double>(value)) {
        new_value = static_cast<int>(std::get<double>(value));
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = std::get<int32_t>(value);
      }

      if (new_value == 2 && _selected_loop != 1) {
        _selected_loop = 2;
        signals_to_emit.insert(&ChannelStripModel::selectedLoopChanged);
      }
    } else if (name == "Looper:loop3") {
      int new_value{};
      if (std::holds_alternative<double>(value)) {
        new_value = static_cast<int>(std::get<double>(value));
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = std::get<int32_t>(value);
      }

      if (new_value == 3 && _selected_loop != 1) {
        _selected_loop = 3;
        signals_to_emit.insert(&ChannelStripModel::selectedLoopChanged);
      }
    } else if (name == "Looper:loop4") {
      int new_value{};
      if (std::holds_alternative<double>(value)) {
        new_value = static_cast<int>(std::get<double>(value));
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = std::get<int32_t>(value);
      }

      if (new_value == 4 && _selected_loop != 1) {
        _selected_loop = 4;
        signals_to_emit.insert(&ChannelStripModel::selectedLoopChanged);
      }
    } else if (name == "Looper:loop5") {
      int new_value{};
      if (std::holds_alternative<double>(value)) {
        new_value = static_cast<int>(std::get<double>(value));
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = std::get<int32_t>(value);
      }

      if (new_value == 5 && _selected_loop != 1) {
        _selected_loop = 5;
        signals_to_emit.insert(&ChannelStripModel::selectedLoopChanged);
      }
    } else if (name == "Looper:loop6") {
      int new_value{};
      if (std::holds_alternative<double>(value)) {
        new_value = static_cast<int>(std::get<double>(value));
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = std::get<int32_t>(value);
      }

      if (new_value == 6 && _selected_loop != 1) {
        _selected_loop = 6;
        signals_to_emit.insert(&ChannelStripModel::selectedLoopChanged);
      }
    } else if (name == "Looper:bars") {
      int new_value{};
      if (std::holds_alternative<int32_t>(value)) {
        new_value = std::get<int32_t>(value);
      } else if (std::holds_alternative<double>(value)) {
        new_value = static_cast<int>(std::get<double>(value));
      }

      if (new_value != _loop_length_in_bars) {
        _loop_length_in_bars = new_value;
        signals_to_emit.insert(&ChannelStripModel::loopLengthInBarsChanged);
      }
    } else if (name == "Looper:mix") {
      double new_value{};
      if (std::holds_alternative<double>(value)) {
        new_value = std::get<double>(value);
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = static_cast<double>(std::get<int32_t>(value));
      }

      if (new_value != _looper_mix) {
        _looper_mix = new_value;
        signals_to_emit.insert(&ChannelStripModel::looperMixChanged);
      }
    } else if (name == "Looper:ENABLED") {} else if (name ==
      "Saturator:bypass") {
      bool enabled = false;
      if (std::holds_alternative<bool>(value)) {
        enabled = std::get<bool>(value);
      } else if (std::holds_alternative<int32_t>(value)) {
        if (std::get<int32_t>(value) == 0) {
          enabled = false;
        } else {
          enabled = true;
        }
      } else if (std::holds_alternative<double>(value)) {
        if (std::get<double>(value) == 0) {
          enabled = false;
        } else {
          enabled = true;
        }
      }

      if (_saturator_bypass_enabled != enabled) {
        _saturator_bypass_enabled = enabled;
        signals_to_emit.insert(
          &ChannelStripModel::saturatorBypassEnabledChanged);
      }
    } else if (name == "Saturator:level_in") {
      double new_value{};

      if (std::holds_alternative<double>(value)) {
        new_value = std::get<double>(value);
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = static_cast<double>(std::get<int32_t>(value));
      }

      if (new_value != _saturator_in_gain) {
        _saturator_in_gain = new_value;
        signals_to_emit.insert(&ChannelStripModel::saturatorInGainChanged);
      }
    } else if (name == "Saturator:drive") {
      double new_value{};

      if (std::holds_alternative<double>(value)) {
        new_value = std::get<double>(value);
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = static_cast<double>(std::get<int32_t>(value));
      }

      if (new_value != _saturator_drive) {
        _saturator_drive = new_value;
        signals_to_emit.insert(&ChannelStripModel::saturatorDriveChanged);
      }
    } else if (name == "Saturator:blend") {
      double new_value{};

      if (std::holds_alternative<double>(value)) {
        new_value = std::get<double>(value);
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = static_cast<double>(std::get<int32_t>(value));
      }

      if (new_value != _saturator_blend) {
        _saturator_blend = new_value;
        signals_to_emit.insert(&ChannelStripModel::saturatorBlendChanged);
      }
    } else if (name == "Saturator:level_out") {
      double new_value{};

      if (std::holds_alternative<double>(value)) {
        new_value = std::get<double>(value);
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = static_cast<double>(std::get<int32_t>(value));
      }

      if (new_value != _saturator_out_gain) {
        _saturator_out_gain = new_value;
        signals_to_emit.insert(&ChannelStripModel::saturatorOutGainChanged);
      }
    } else if (name == "Compressor:bypass") {
      bool new_value{};

      if (std::holds_alternative<bool>(value)) {
        new_value = std::get<bool>(value);
      } else if (std::holds_alternative<int32_t>(value)) {
        if (std::get<int32_t>(value) == 0) {
          new_value = false;
        } else {
          new_value = true;
        }
      } else if (std::holds_alternative<double>(value)) {
        if (std::get<double>(value) == 0) {
          new_value = false;
        } else {
          new_value = true;
        }
      }

      if (new_value != _compressor_bypass_enabled) {
        _compressor_bypass_enabled = new_value;
        signals_to_emit.insert(
          &ChannelStripModel::compressorBypassEnabledChanged);
      }
    } else if (name == "Compressor:threshold") {
      double new_value{};

      if (std::holds_alternative<double>(value)) {
        new_value = std::get<double>(value);
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = static_cast<double>(std::get<int32_t>(value));
      }

      if (new_value != _compressor_threshold) {
        _compressor_threshold = new_value;
        signals_to_emit.insert(&ChannelStripModel::compressorThresholdChanged);
      }
    } else if (name == "Compressor:ratio") {
      double new_value{};

      if (std::holds_alternative<double>(value)) {
        new_value = std::get<double>(value);
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = static_cast<double>(std::get<int32_t>(value));
      }

      if (new_value != _compressor_ratio) {
        _compressor_ratio = new_value;
        signals_to_emit.insert(&ChannelStripModel::compressorRatioChanged);
      }
    } else if (name == "Compressor:attack") {
      double new_value{};

      if (std::holds_alternative<double>(value)) {
        new_value = std::get<double>(value);
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = static_cast<double>(std::get<int32_t>(value));
      }

      if (new_value != _compressor_attack) {
        _compressor_attack = new_value;
        signals_to_emit.insert(&ChannelStripModel::compressorAttackChanged);
      }
    } else if (name == "Compressor:release") {
      double new_value{};

      if (std::holds_alternative<double>(value)) {
        new_value = std::get<double>(value);
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = static_cast<double>(std::get<int32_t>(value));
      }

      if (new_value != _compressor_release) {
        _compressor_release = new_value;
        signals_to_emit.insert(&ChannelStripModel::compressorReleaseChanged);
      }
    } else if (name == "Compressor:knee") {
      double new_value{};

      if (std::holds_alternative<double>(value)) {
        new_value = std::get<double>(value);
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = static_cast<double>(std::get<int32_t>(value));
      }

      if (new_value != _compressor_knee) {
        _compressor_knee = new_value;
        signals_to_emit.insert(&ChannelStripModel::compressorKneeChanged);
      }
    } else if (name == "Compressor:mix") {
      double new_value{};

      if (std::holds_alternative<double>(value)) {
        new_value = std::get<double>(value);
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = static_cast<double>(std::get<int32_t>(value));
      }

      if (new_value != _compressor_mix) {
        _compressor_mix = new_value;
        signals_to_emit.insert(&ChannelStripModel::compressorMixChanged);
      }
    } else if (name == "Compressor:makeup") {
      double new_value{};

      if (std::holds_alternative<double>(value)) {
        new_value = std::get<double>(value);
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = static_cast<double>(std::get<int32_t>(value));
      }

      if (new_value != _compressor_gain) {
        _compressor_gain = new_value;
        signals_to_emit.insert(&ChannelStripModel::compressorGainChanged);
      }
    } else if (name == "Equalizer:high") {
      double new_value{};

      if (std::holds_alternative<double>(value)) {
        new_value = std::get<double>(value);
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = static_cast<double>(std::get<int32_t>(value));
      }

      if (new_value != _equalizer_high_gain) {
        _equalizer_high_gain = new_value;
        signals_to_emit.insert(&ChannelStripModel::equalizerHighGainChanged);
      }
    } else if (name == "Equalizer:mid") {
      double new_value{};

      if (std::holds_alternative<double>(value)) {
        new_value = std::get<double>(value);
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = static_cast<double>(std::get<int32_t>(value));
      }

      if (new_value != _equalizer_mid_gain) {
        _equalizer_mid_gain = new_value;
        signals_to_emit.insert(&ChannelStripModel::equalizerMidGainChanged);
      }
    } else if (name == "Equalizer:low") {
      double new_value{};

      if (std::holds_alternative<double>(value)) {
        new_value = std::get<double>(value);
      } else if (std::holds_alternative<int32_t>(value)) {
        new_value = static_cast<double>(std::get<int32_t>(value));
      }

      if (new_value != _equalizer_low_gain) {
        _equalizer_low_gain = new_value;
        signals_to_emit.insert(&ChannelStripModel::equalizerLowGainChanged);
      }
    }
  }

  for (auto &signal : signals_to_emit) {
    Q_EMIT signal(this);
  }
}
