import QtQuick
import QtCore
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Templates
import org.kde.kdeconnect

ApplicationWindow {
    id: window
    width: 640
    height: 480
    visible: true
    title: qsTr("KdeConnect")

     ColumnLayout {
        ScrollView {
                        Layout.fillWidth: true
                        ListView {
                            id: devices
                            Layout.fillWidth: true
                            clip: true

                            section {
                                property: "status"
                                delegate:  {
                                    width: ListView.view.width
                                    text: switch (parseInt(section)) {
                                    case DevicesModel.Paired:
                                        return i18nd("kdeconnect-app", "Remembered");
                                    case DevicesModel.Reachable:
                                        return i18nd("kdeconnect-app", "Available");
                                    case (DevicesModel.Reachable | DevicesModel.Paired):
                                        return i18nd("kdeconnect-app", "Connected");
                                    }
                                }
                            }

                            model: DevicesModel {

                            }
                            delegate: ItemDelegate {
                                id: delegate
                                icon.name: iconName
                                text: model.name
                                width: ListView.view.width
                                highlighted: false


                            }
                        }
                    }
    }

    Settings {
        id: settings
        property alias windowX: window.x
        property alias windowY: window.y
        property alias windowWidth: window.width
        property alias windowHeight: window.height

    }
}
