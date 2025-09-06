import QtQuick
import QtQuick.Layouts
import QtQuick.Controls 2 as QQC2
import org.kde.kirigami 2 as Kirigami
import org.kde.kitemmodels 1.0 as KItemModels

Item {
    id: root
    required property ChannelStripModel model
    required property string channelType
    required property string title

    implicitWidth: 240
    implicitHeight: delegateLayout.implicitHeight
    property var dialWidth: 48

    ColumnLayout {
        id: delegateLayout
        anchors.fill: parent
        Kirigami.Heading {
            level: 3
            text: root.channelType === "Input" ? root.title : model.name
            Layout.alignment: Qt.AlignHCenter
        }

        QQC2.GroupBox {
            id: inputRoutingGroupBox
            title: "Input Routing"
            visible: root.channelType === "Input"
            Layout.fillWidth: true

            KItemModels.KSortFilterProxyModel {
                id: filteredNodes
                sourceModel: Nodes
                filterRoleName: "mediaClass"
                filterRegularExpression: /^Audio\/Source|Stream\/Output\/Audio$/
                sortRoleName: "name"
                sortOrder: Qt.AscendingOrder
            }

            ColumnLayout {
                id: inputRoutingColumnLayout
                width: parent.availableWidth

                QQC2.Label {
                    text: root.model.objectId ? root.model.objectId : ""
                }

                QQC2.ComboBox {
                    id: targetCombo
                    model: filteredNodes
                    textRole: "name"
                    Layout.fillWidth: true
                    Layout.maximumWidth: inputRoutingGroupBox.availableWidth

                    function rowForName(name) {
                        if (!name || name === "") return -1
                        for (let r = 0; r < count; ++r) {
                            if (String(textAt(r)) === String(name))
                                return r
                        }
                        return -1
                    }

                    function updateSelection() {
                        currentIndex = rowForName(root.model.targetObject)
                    }

                    Component.onCompleted: { updateSelection() }

                    onActivated: {
                        root.model.targetObject = currentText
                    }

                    Connections {
                        target: root.model
                        function targetObjectChanged() { targetCombo.updateSelection() }
                    }
                }
            }
        }

        QQC2.GroupBox {
            id: looperGroupBox
            title: "Looper"
            visible: root.channelType === "Input" || root.channelType === "Group"
            Layout.fillWidth: true

            ColumnLayout {
                RowLayout {
                    Layout.fillWidth: true

                    QQC2.ButtonGroup {
                        id: loopSelector; exclusive: true
                    }

                    QQC2.ToolButton {
                        text: "1"
                        checkable: true
                        QQC2.ButtonGroup.group: loopSelector
                        property int data: 1
                        checked: root.model.selectedLoop === data
                        onClicked: root.model.selectedLoop = data
                    }

                    QQC2.ToolButton {
                        text: "2"
                        checkable: true
                        QQC2.ButtonGroup.group: loopSelector
                        property int data: 2
                        checked: root.model.selectedLoop === data
                        onClicked: root.model.selectedLoop = data
                    }

                    QQC2.ToolButton {
                        text: "3"
                        checkable: true
                        QQC2.ButtonGroup.group: loopSelector
                        property int data: 3
                        checked: root.model.selectedLoop === data
                        onClicked: root.model.selectedLoop = data
                    }

                    QQC2.ToolButton {
                        text: "4"
                        checkable: true
                        QQC2.ButtonGroup.group: loopSelector
                        property int data: 4
                        checked: root.model.selectedLoop === data
                        onClicked: root.model.selectedLoop = data
                    }

                    QQC2.ToolButton {
                        text: "5"
                        checkable: true
                        QQC2.ButtonGroup.group: loopSelector
                        property int data: 5
                        checked: root.model.selectedLoop === data
                        onClicked: root.model.selectedLoop = data
                    }

                    QQC2.ToolButton {
                        text: "6"
                        checkable: true
                        QQC2.ButtonGroup.group: loopSelector
                        property int data: 6
                        checked: root.model.selectedLoop === data
                        onClicked: root.model.selectedLoop = data
                    }
                }

                GridLayout {
                    columns: 2
                    QQC2.Label {
                        text: i18nc("@label/number_of_bars", "#Bars")
                    }

                    // Bars
                    QQC2.SpinBox {
                        editable: true; Layout.fillWidth: true
                        from: 1;
                        to: 32
                        value: root.model.loopLengthInBars
                        onValueModified: {
                            if (value !== root.model.loopLengthInBars) {
                                root.model.loopLengthInBars = value
                            }
                        }
                    }
                }

                GridLayout {
                    columns: 4

                    QQC2.ToolButton {
                        text: "Play"; checkable: true
                        checked: root.model.looperPlaying
                        onClicked: {
                            if (root.model.looperPlaying !== checked) {
                                root.model.looperPlaying = checked
                            }
                        }
                    }

                    QQC2.ToolButton {
                        text: "Record"; checkable: true
                        checked: root.model.looperRecording
                        onClicked: {
                            if (root.model.looperRecording !== checked) {
                                root.model.looperRecording = checked
                            }
                        }
                    }

                    // Looper Mix
                    QQC2.Dial {
                        inputMode: QQC2.Dial.Vertical
                        Layout.columnSpan: 1
                        implicitWidth: root.dialWidth
                        implicitHeight: root.dialWidth
                        from: 0.0
                        to: 100.0
                        live: true
                        value: root.model.looperMix
                        onValueChanged: {
                            if (pressed && value !== root.model.looperMix) {
                                root.model.looperMix = value
                            }
                        }
                    }

                    QQC2.Label {
                        text: "Mix"
                    }
                }
            }
        }

        QQC2.GroupBox {
            id: saturatorGroupBox
            title: "Saturator"
            Layout.fillWidth: true

            GridLayout {
                columns: 4

                // Saturator bypassed
                QQC2.ToolButton {
                    text: "Bypass"; checkable: true;
                    checked: root.model.saturatorBypassEnabled
                    onClicked: {
                        if (root.model.saturatorBypassEnabled !== checked) {
                            root.model.saturatorBypassEnabled = checked
                        }
                    }
                    Layout.columnSpan: 4
                }

                // In Gain
                QQC2.Dial {
                    inputMode: QQC2.Dial.Vertical; implicitWidth: root.dialWidth
                    implicitHeight: root.dialWidth; from: 0.015625;
                    to: 64.0
                    live: true
                    value: root.model.saturatorInGain
                    onValueChanged: {
                        if (pressed && value !== root.model.saturatorInGain) {
                            root.model.saturatorInGain = value
                        }
                    }
                }

                // Drive
                QQC2.Dial {
                    inputMode: QQC2.Dial.Vertical; implicitWidth: root.dialWidth;
                    implicitHeight: root.dialWidth; from: 0.1;
                    to: 10.0
                    live: true
                    value: root.model.saturatorDrive
                    onValueChanged: {
                        if (pressed && value !== root.model.saturatorDrive) {
                            root.model.saturatorDrive = value
                        }
                    }
                }

                // Blend
                QQC2.Dial {
                    inputMode: QQC2.Dial.Vertical; implicitWidth: root.dialWidth;
                    implicitHeight: root.dialWidth; from: -10.0;
                    to: 10.0
                    live: true
                    value: root.model.saturatorBlend
                    onValueChanged: {
                        if (pressed && value !== root.model.saturatorBlend) {
                            root.model.saturatorDrive = value
                        }
                    }
                }

                // Out
                QQC2.Dial {
                    inputMode: QQC2.Dial.Vertical; implicitWidth: root.dialWidth;
                    implicitHeight: root.dialWidth; from: 0.015625;
                    to: 64.0
                    live: true
                    value: root.model.saturatorOutGain
                    onValueChanged: {
                        if (pressed && value !== root.model.saturatorOutGain) {
                            root.model.saturatorOutGain = value
                        }
                    }
                }

                QQC2.Label {
                    text: "In Gain"
                }
                QQC2.Label {
                    text: "Drive"
                }
                QQC2.Label {
                    text: "Blend"
                }
                QQC2.Label {
                    text: "Out Gain"
                }
            }
        }

        QQC2.GroupBox {
            id: compressorGroupBox
            title: "Compressor"
            Layout.fillWidth: true

            GridLayout {
                width: parent.availableWidth
                columns: 4

                QQC2.ToolButton {
                    text: "Bypass"; checkable: true; Layout.columnSpan: 4
                    checked: root.model.compressorBypassEnabled
                    onClicked: {
                        if (root.model.compressorBypassEnabled !== checked) {
                            root.model.compressorBypassEnabled = checked
                        }
                    }
                }

                // Thres
                QQC2.Dial {
                    inputMode: QQC2.Dial.Vertical; implicitWidth: root.dialWidth;
                    implicitHeight: root.dialWidth; from: 0.000977;
                    to: 1.0
                    live: true
                    value: root.model.compressorThreshold
                    onValueChanged: {
                        if (pressed && value !== root.model.compressorThreshold) {
                            root.model.compressorThreshold = value
                        }
                    }
                }

                // Ratio
                QQC2.Dial {
                    inputMode: QQC2.Dial.Vertical; implicitWidth: root.dialWidth;
                    implicitHeight: root.dialWidth; from: 1.0;
                    to: 20.0
                    live: true
                    value: root.model.compressorRatio
                    onValueChanged: {
                        if (pressed && value !== root.model.compressorRatio) {
                            root.model.compressorRatio = value
                        }
                    }
                }

                // Attack
                QQC2.Dial {
                    inputMode: QQC2.Dial.Vertical; implicitWidth: root.dialWidth;
                    implicitHeight: root.dialWidth; from: 0.01;
                    to: 2000.0
                    live: true
                    value: root.model.compressorAttack
                    onValueChanged: {
                        if (pressed && value !== root.model.compressorAttack) {
                            root.model.compressorAttack = value
                        }
                    }
                }

                // Release
                QQC2.Dial {
                    inputMode: QQC2.Dial.Vertical; implicitWidth: root.dialWidth;
                    implicitHeight: root.dialWidth; from: 0.01;
                    to: 2000.0
                    live: true
                    value: root.model.compressorRelease
                    onValueChanged: {
                        if (pressed && value !== root.model.compressorRelease) {
                            root.model.compressorRelease = value
                        }
                    }
                }

                QQC2.Label {
                    text: "Thres"
                }
                QQC2.Label {
                    text: "Ratio"
                }
                QQC2.Label {
                    text: "Attack"
                }
                QQC2.Label {
                    text: "Release"
                }

                // Knee
                QQC2.Dial {
                    inputMode: QQC2.Dial.Vertical; implicitWidth: root.dialWidth;
                    implicitHeight: root.dialWidth; from: 1.0;
                    to: 8.0
                    live: true
                    value: root.model.compressorKnee
                    onValueChanged: {
                        if (pressed && value !== root.model.compressorKnee) {
                            root.model.compressorKnee = value
                        }
                    }
                }

                // Mix
                QQC2.Dial {
                    inputMode: QQC2.Dial.Vertical; implicitWidth: root.dialWidth;
                    implicitHeight: root.dialWidth; from: 0.0;
                    to: 1.0
                    live: true
                    value: root.model.compressorMix
                    onValueChanged: {
                        if (pressed && value !== root.model.compressorMix) {
                            root.model.compressorMix = value
                        }
                    }
                }

                // Gain
                QQC2.Dial {
                    id:
                        compressorGainDial; inputMode: QQC2.Dial.Vertical; implicitWidth: root.dialWidth;
                    implicitHeight: root.dialWidth; from: 1.0;
                    to: 64.0
                    live: true
                    value: root.model.compressorGain
                    onValueChanged: {
                        if (pressed && value !== root.model.compressorGain) {
                            root.model.compressorGain = value
                        }
                    }
                }

                Item {
                    width: compressorGainDial.implicitWidth; height: compressorGainDial.implicitHeight
                }

                QQC2.Label {
                    text: "Knee"
                }
                QQC2.Label {
                    text: "Mix"
                }
                QQC2.Label {
                    text: "Makeup"
                }
            }
        }

        RowLayout {
            QQC2.GroupBox {
                id: equalizerGroupBox
                title: "Equalizer"
                Layout.fillHeight: true
                Layout.fillWidth: true

                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter

                    QQC2.Dial {
                        inputMode: QQC2.Dial.Vertical; implicitWidth: 56; implicitHeight: 56
                        from: -24.0;
                        to: 24.0
                        live: true
                        value: root.model.equalizerHighGain
                        onValueChanged: {
                            if (pressed && value !== root.model.equalizerHighGain) {
                                root.model.equalizerHighGain = value
                            }
                        }
                        Layout.alignment: Qt.AlignHCenter
                    }

                    QQC2.Label {
                        text: "High"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    QQC2.Dial {
                        inputMode: QQC2.Dial.Vertical; implicitWidth: 56; implicitHeight: 56
                        from: -24.0;
                        to: 24.0
                        live: true
                        value: root.model.equalizerMidGain
                        onValueChanged: {
                            if (pressed && value !== root.model.equalizerMidGain) {
                                root.model.equalizerMidGain = value
                            }
                        }
                        Layout.alignment: Qt.AlignHCenter
                    }

                    QQC2.Label {
                        text: "Mid"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    QQC2.Dial {
                        inputMode: QQC2.Dial.Vertical; implicitWidth: 56; implicitHeight: 56
                        from: -24.0;
                        to: 24.0
                        live: true
                        value: root.model.equalizerLowGain
                        onValueChanged: {
                            if (pressed && value !== root.model.equalizerLowGain) {
                                root.model.equalizerLowGain = value
                            }
                        }
                        Layout.alignment: Qt.AlignHCenter
                    }

                    QQC2.Label {
                        text: "Low"
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            QQC2.GroupBox {
                id: volumeGroupBox
                title: "Volume"
                Layout.fillHeight: true
                Layout.fillWidth: true

                ColumnLayout {
                    anchors.fill: parent

                    QQC2.Slider {
                        id: volumeSlider
                        orientation: Qt.Vertical
                        from: 0.0
                        to: 10.0
                        live: true
                        value: root.model.volume
                        onValueChanged: {
                            if (pressed && value !== root.model.volume) {
                                root.model.volume = value
                            }
                        }
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignHCenter
                    }

                    QQC2.Slider {
                        id: panSlider
                        orientation: Qt.Horizontal
                        to: 1.0
                        from: -1.0
                        live: true
                        value: root.model.pan
                        onValueChanged: {
                            if (pressed && value !== root.model.pan) {
                                root.model.pan = value
                            }
                        }
                        Layout.fillWidth: true
                    }
                }
            }
        }

        QQC2.GroupBox {
            id: outputRoutingGroupBox
            title: "Output Routing"
            Layout.fillWidth: true
            visible: root.channelType === "Input"

            ColumnLayout {
                width: parent.availableWidth
                QQC2.ComboBox {
                    model: ["Drums", "Bass", "Melody", "Atmos"]
                    Layout.fillWidth: true
                }
            }
        }
    }
}
