import QtQuick
import QtCore
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kdeconnect

ApplicationWindow {
    id: root
    width: 300
    height: 200
    visible: true
    modality: Qt.WindowModal
    title: qsTr("Settings")

    DBusProperty {
        id: announcedNameProperty
        object: DaemonDBusInterface
        read: "announcedName"
        defaultValue: qsTr("DeviceName")
    }

    ColumnLayout {
        spacing: 2
        anchors.centerIn: parent

        Label{
            text: qsTr("Device name")
        }
        TextField {
            id: deviceName
            text: announcedNameProperty.value
        }
    }

    footer: DialogButtonBox {
        standardButtons: DialogButtonBox.Apply | DialogButtonBox.Cancel

        onApplied: {
            DaemonDBusInterface.setAnnouncedName(deviceName.text)
            close()
        }

        onRejected: close()
    }

}

