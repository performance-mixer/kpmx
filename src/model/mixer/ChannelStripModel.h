#pragma once

#include <QVariant>
#include <optional>
#include <string>
#include <vector>
#include <qqmlintegration.h>
#include <spa/pod/pod.h>

#include "synchronization/lock_free_queue.h"
#include "wireplumber/wireplumber_object_manager_thread.h"

class ChannelStripModel : public QObject {
  Q_OBJECT

  Q_PROPERTY(
    QVariant targetObject READ target_object WRITE setTargetObject NOTIFY
    targetObjectChanged)
  Q_PROPERTY(QVariant objectId READ object_id WRITE setObjectId NOTIFY objectIdChanged)
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
  Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
  Q_PROPERTY(double pan READ pan WRITE setPan NOTIFY panChanged)
  Q_PROPERTY(
    uint selectedLoop READ selected_loop WRITE setSelectedLoop NOTIFY
    selectedLoopChanged)
  Q_PROPERTY(
    uint loopLengthInBars READ loop_length_in_bars WRITE setLoopLengthInBars
    NOTIFY loopLengthInBarsChanged)
  Q_PROPERTY(
    bool looperPlaying READ looper_playing WRITE setLooperPlaying NOTIFY
    looperPlayingChanged)
  Q_PROPERTY(
    bool looperRecording READ looper_recording WRITE setLooperRecording NOTIFY
    looperRecordingChanged)
  Q_PROPERTY(
    double looperMix READ looper_mix WRITE setLooperMix NOTIFY looperMixChanged)
  Q_PROPERTY(
    bool saturatorBypassEnabled READ saturator_bypass_enabled WRITE
    setSaturatorBypassEnabled NOTIFY saturatorBypassEnabledChanged)
  Q_PROPERTY(
    double saturatorInGain READ saturator_in_gain WRITE setSaturatorInGain
    NOTIFY saturatorInGainChanged)
  Q_PROPERTY(
    double saturatorDrive READ saturator_drive WRITE setSaturatorDrive NOTIFY
    saturatorDriveChanged)
  Q_PROPERTY(
    double saturatorBlend READ saturator_blend WRITE setSaturatorBlend NOTIFY
    saturatorBlendChanged)
  Q_PROPERTY(
    double saturatorOutGain READ saturator_out_gain WRITE setSaturatorOutGain
    NOTIFY saturatorOutGainChanged)
  Q_PROPERTY(
    bool compressorBypassEnabled READ compressor_bypass_enabled WRITE
    setCompressorBypassEnabled NOTIFY compressorBypassEnabledChanged)
  Q_PROPERTY(
    double compressorThreshold READ compressor_threshold WRITE
    setCompressorThreshold NOTIFY compressorThresholdChanged)
  Q_PROPERTY(
    double compressorRatio READ compressor_ratio WRITE setCompressorRatio NOTIFY
    compressorRatioChanged)
  Q_PROPERTY(
    double compressorAttack READ compressor_attack WRITE setCompressorAttack
    NOTIFY compressorAttackChanged)
  Q_PROPERTY(
    double compressorRelease READ compressor_release WRITE setCompressorRelease
    NOTIFY compressorReleaseChanged)
  Q_PROPERTY(
    double compressorKnee READ compressor_knee WRITE setCompressorKnee NOTIFY
    compressorKneeChanged)
  Q_PROPERTY(
    double compressorMix READ compressor_mix WRITE setCompressorMix NOTIFY
    compressorMixChanged)
  Q_PROPERTY(
    double compressorGain READ compressor_gain WRITE setCompressorGain NOTIFY
    compressorGainChanged)
  Q_PROPERTY(
    double equalizerHighGain READ equalizer_high_gain WRITE setEqualizerHighGain
    NOTIFY equalizerHighGainChanged)
  Q_PROPERTY(
    double equalizerMidGain READ equalizer_mid_gain WRITE setEqualizerMidGain
    NOTIFY equalizerMidGainChanged)
  Q_PROPERTY(
    double equalizerLowGain READ equalizer_low_gain WRITE setEqualizerLowGain
    NOTIFY equalizerLowGainChanged)

public:
  ChannelStripModel(const lock_free_queue &queue,
                             ::metadata_updates_queue metadata_updates_queue,
                             std::atomic<bool> &has_value,
                             QObject *parent = nullptr);

  [[nodiscard]] QVariant target_object() const;
  void setTargetObject(QVariant target_object);
  void setTargetObject(const std::optional<std::string> &target_object);

  [[nodiscard]] QVariant object_id() const;
  void setObjectId(const QVariant &object_id);
  void setObjectId(std::optional<int> object_id);

  [[nodiscard]] uint selected_loop() const;
  void setSelectedLoop(uint selected_loop);

  [[nodiscard]] uint loop_length_in_bars() const;
  void setLoopLengthInBars(uint loop_length_in_bars);

  [[nodiscard]] bool looper_playing() const;
  void setLooperPlaying(bool looper_playing);

  [[nodiscard]] bool looper_recording() const;
  void setLooperRecording(bool looper_recording);

  [[nodiscard]] double looper_mix() const;
  void setLooperMix(double looper_mix);

  [[nodiscard]] bool saturator_bypass_enabled() const;
  void setSaturatorBypassEnabled(bool saturator_bypass_enabled);

  [[nodiscard]] double saturator_in_gain() const;
  void setSaturatorInGain(double saturator_in_gain);

  [[nodiscard]] double saturator_drive() const;
  void setSaturatorDrive(double saturator_drive);

  [[nodiscard]] double saturator_blend() const;
  void setSaturatorBlend(double saturator_blend);

  [[nodiscard]] double saturator_out_gain() const;
  void setSaturatorOutGain(double saturator_out_gain);

  [[nodiscard]] bool compressor_bypass_enabled() const;
  void setCompressorBypassEnabled(bool compressor_bypass_enabled);

  [[nodiscard]] double compressor_threshold() const;
  void setCompressorThreshold(double compressor_threshold);

  [[nodiscard]] double compressor_ratio() const;
  void setCompressorRatio(double compressor_ratio);

  [[nodiscard]] double compressor_attack() const;
  void setCompressorAttack(double compressor_attack);

  [[nodiscard]] double compressor_release() const;
  void setCompressorRelease(double compressor_release);

  [[nodiscard]] double compressor_knee() const;
  void setCompressorKnee(double compressor_knee);

  [[nodiscard]] double compressor_mix() const;
  void setCompressorMix(double compressor_mix);

  [[nodiscard]] double compressor_gain() const;
  void setCompressorGain(double compressor_gain);

  [[nodiscard]] double equalizer_high_gain() const;
  void setEqualizerHighGain(double equalizer_high_gain);

  [[nodiscard]] double equalizer_mid_gain() const;
  void setEqualizerMidGain(double equalizer_mid_gain);

  [[nodiscard]] double equalizer_low_gain() const;
  void setEqualizerLowGain(double equalizer_low_gain);

  [[nodiscard]] QString name() const;
  void setName(const QString &name);

  [[nodiscard]] double volume() const;
  void setVolume(double volume);

  [[nodiscard]] double pan() const;
  void setPan(double pan);

  void apply(spa_pod *control_pod);
  void parse_spa_prop_params(const spa_pod *params_pod);

private:
  std::optional<std::string> _target_object;
  std::optional<int> _object_id;
  uint _selected_loop = 1;
  uint _loop_length_in_bars = 2;
  bool _looper_playing = false;
  bool _looper_recording = false;
  double _looper_mix = 50.0;
  bool _saturator_bypass_enabled = false;
  double _saturator_in_gain = 1.0;
  double _saturator_drive = 5.0;
  double _saturator_blend = 10.0;
  double _saturator_out_gain = 1.0;
  bool _compressor_bypass_enabled = false;
  double _compressor_threshold = 0.125;
  double _compressor_ratio = 2.0;
  double _compressor_attack = 20.0;
  double _compressor_release = 250.0;
  double _compressor_knee = 2.82843;
  double _compressor_mix = 1.0;
  double _compressor_gain = 1.0;
  double _equalizer_high_gain = 0.0;
  double _equalizer_mid_gain = 0.0;
  double _equalizer_low_gain = 0.0;
  QString _name;
  double _volume = 0.0;
  double _pan = 0.0;

  Q_SIGNAL void selectedLoopChanged();
  Q_SIGNAL void loopLengthInBarsChanged();
  Q_SIGNAL void looperPlayingChanged();
  Q_SIGNAL void looperRecordingChanged();
  Q_SIGNAL void looperMixChanged();
  Q_SIGNAL void saturatorBypassEnabledChanged();
  Q_SIGNAL void saturatorInGainChanged();
  Q_SIGNAL void saturatorDriveChanged();
  Q_SIGNAL void saturatorBlendChanged();
  Q_SIGNAL void saturatorOutGainChanged();
  Q_SIGNAL void compressorBypassEnabledChanged();
  Q_SIGNAL void compressorThresholdChanged();
  Q_SIGNAL void compressorAttackChanged();
  Q_SIGNAL void compressorRatioChanged();
  Q_SIGNAL void compressorReleaseChanged();
  Q_SIGNAL void compressorKneeChanged();
  Q_SIGNAL void compressorMixChanged();
  Q_SIGNAL void compressorGainChanged();
  Q_SIGNAL void equalizerHighGainChanged();
  Q_SIGNAL void equalizerMidGainChanged();
  Q_SIGNAL void equalizerLowGainChanged();
  Q_SIGNAL void nameChanged();
  Q_SIGNAL void volumeChanged();
  Q_SIGNAL void panChanged();
  Q_SIGNAL void targetObjectChanged();
  Q_SIGNAL void objectIdChanged();

  Q_DISABLE_COPY(ChannelStripModel)

  lock_free_queue _queue;
  ::metadata_updates_queue _metadata_updates_queue;
  std::atomic<bool> &_has_value;
};

Q_DECLARE_METATYPE(ChannelStripModel*)
