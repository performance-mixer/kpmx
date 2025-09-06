import QtQuick
import QtQuick.Layouts
import QtQuick.Controls 2 as QQC2
import org.kde.kirigami 2 as Kirigami
import org.performance_mixer.kpmx

Kirigami.ScrollablePage {
    id: root
    title: "Nodes"
    Layout.fillWidth: true
    Layout.fillHeight: true

    Component {
        id: listViewDelegate

        Item {
            id: listItemRoot
            width: mainList.width - mainList.leftMargin - mainList.rightMargin
            height: itemLayout.implicitHeight + Kirigami.Units.smallSpacing * 2
            required property string name
            required property int id
            required property int objectSerial
            required property string mediaClass
            required property string targetObject

            RowLayout {
                id: itemLayout

                GridLayout {
                    id: gridLayout
                    columns: 3

                    QQC2.Label {
                        text: id
                    }

                    QQC2.Label {
                        text: objectSerial
                    }

                    QQC2.Label {
                        text: targetObject
                    }

                    QQC2.Label {
                        text: mediaClass
                    }

                    QQC2.Label {
                        text: name
                        Layout.columnSpan: 3
                    }
                }
            }
        }
    }

    ListView {
        id: mainList
        anchors.fill: parent
        model: Nodes
        delegate: listViewDelegate
    }

    flickable: mainList
}
