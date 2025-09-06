import QtQuick
import QtQuick.Layouts
import QtQuick.Controls 2 as QQC2
import org.kde.kirigami 2 as Kirigami
import org.performance_mixer.kpmx

Kirigami.Page {
    title: "Layer Mixer"
    id: inputChannelsPage
    Layout.fillHeight: true
    Layout.fillWidth: true

    property color backgroundColor: "#29323c"

    Kirigami.OverlayDrawer {
        id: nodesDrawer
        edge: Qt.RightEdge
        modal: false
        width: 600
        contentItem: ColumnLayout {
            anchors.fill: parent

            NodesListView {}

            QQC2.Button {
                text: "Close"
                onClicked: nodesDrawer.close()
            }
        }
    }

    ListView {
        id: pager
        anchors.fill: parent
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        boundsBehavior: Flickable.StopAtBounds
        spacing: Kirigami.Units.smallSpacing
        model: Mixer.layers
        highlightMoveDuration: 100
        highlightResizeDuration: 0

        delegate: LayerPage {
            required property string name
            required property var mixerLayer
            title: name
            inputChannels: mixerLayer.inputChannels
            groupChannels: mixerLayer.groupChannels
            width: pager.width
            height: pager.height
        }
    }

    footer: Rectangle {
        color: "transparent"
        height: Kirigami.Units.iconSizes.medium + Kirigami.Units.smallSpacing * 2

        RowLayout {
            id: btnRow
            anchors.fill: parent
            anchors.margins: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.smallSpacing

            Repeater {
                model: Mixer.layers

                delegate: QQC2.ToolButton {
                    required property string name
                    required property int index

                    text: name
                    checkable: true
                    checked: pager.currentIndex === index
                    onClicked: pager.currentIndex = index
                }
            }

            Item { Layout.fillWidth: true }
        }
    }
}