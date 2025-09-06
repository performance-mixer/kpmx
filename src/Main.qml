import QtQuick
import QtQuick.Layouts
import QtQuick.Controls 2 as QQC2
import org.kde.kirigami 2 as Kirigami
import org.performance_mixer.kpmx

Kirigami.ApplicationWindow {
    id: root
    visibility: Window.Maximized
    title: i18nc("@title:window", "Performance Mixer")
    pageStack.defaultColumnWidth: root.width
    pageStack.initialPage: LayerMixer { }
}
