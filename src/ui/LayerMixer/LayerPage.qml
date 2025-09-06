import QtQuick
import QtQuick.Layouts
import QtQuick.Controls 2 as QQC2
import org.kde.kirigami 2 as Kirigami
import org.performance_mixer.kpmx

Kirigami.Page {
    id: root
    required property ChannelStripListModel inputChannels
    required property ChannelStripListModel groupChannels
    property color backgroundColor: "#29323c"

    background: Rectangle {
        color: root.backgroundColor
        QQC2.Label {
            anchors.centerIn: parent
            text: root.title
            color: Qt.darker(root.backgroundColor, 1.5)
            font.pixelSize: 40
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        QQC2.ScrollView {
            id: scrollView
            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true

            RowLayout {
                spacing: Kirigami.Units.smallSpacing
                height: scrollView.height

                Repeater {
                    model: root.inputChannels

                    delegate: Kirigami.AbstractCard
                    {
                        Layout.fillHeight: true
                        Layout.fillWidth: false
                        Layout.preferredWidth: contentItem.implicitWidth + leftPadding + rightPadding
                        Layout.maximumWidth: Layout.preferredWidth
                        required property var channelStrip

                        contentItem: ChannelStrip {
                            model: channelStrip
                            channelType: "Input"
                            title: "Input"
                        }
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: false
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignRight

            RowLayout {
                Layout.fillHeight: true

                Repeater {
                    model: root.groupChannels

                    delegate: Kirigami.AbstractCard
                    {
                        Layout.fillHeight: true
                        Layout.fillWidth: false
                        Layout.preferredWidth: contentItem.implicitWidth + leftPadding + rightPadding
                        Layout.maximumWidth: Layout.preferredWidth
                        required property var channelStrip

                        contentItem: ChannelStrip {
                            model: channelStrip
                            channelType: "Group"
                            title: "Group"
                        }
                    }
                }
            }
        }
    }
}
