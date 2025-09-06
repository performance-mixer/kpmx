add_executable(kpmx)

ecm_add_qml_module(kpmx
        URI
        org.performance_mixer.kpmx
)

target_sources(kpmx
        PRIVATE
        main.cpp
        model/mixer/ChannelStripModel.cpp
        model/mixer/ChannelStripListModel.cpp
        model/mixer/LayerModel.cpp
        model/mixer/LayersListModel.cpp
        model/mixer/PerformanceMixerModel.cpp
        model/pipewire/NodesListModel.cpp
        pipewire/pipewire_monitor_thread.cpp
        pipewire/pipewire_control_thread.cpp
        pipewire/pipewire_feedback_thread.cpp
        synchronization/updates_to_qt_thread.cpp
        synchronization/updates_to_qt_nodes_model_thread.cpp
        wireplumber/wireplumber_object_manager_thread.cpp
        setup/mixer_setup.cpp
        boost/exception.cpp
)

ecm_target_qml_sources(kpmx
        SOURCES
        Main.qml
        ui/MixerView/ChannelStrip.qml
        ui/LayerMixer/LayerMixer.qml
        ui/LayerMixer/LayerPage.qml
        ui/NodesListView/NodesListView.qml
)

target_link_libraries(kpmx
        PRIVATE
        Qt6::Quick
        Qt6::Qml
        Qt6::Gui
        Qt6::QuickControls2
        Qt6::Widgets
        KF6::I18n
        KF6::CoreAddons
        KF6::IconThemes
        PkgConfig::GLIB
        PkgConfig::GOBJECT
        PkgConfig::WIREPLUMBER
        PkgConfig::PIPEWIRE
        Boost::boost
)

install(TARGETS kpmx ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
