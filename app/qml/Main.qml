import QtQuick
import QtCore
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kdeconnect

ApplicationWindow {
    id: window
    width: 640
    height: 480
    minimumWidth: 300
    minimumHeight: 400
    visible: true
    title: qsTr("KdeConnect")

    header: ToolBar{
        height: 40
        contentItem: RowLayout {
            Label{
                text: qsTr("Devices")
                font.pixelSize: 16
                Layout.fillWidth: true
            }

            ToolButton{
                icon.name: 'view-refresh'
                text: qsTr("Refresh")
                ToolTip.text:  text
                ToolTip.visible: hovered
                onClicked: DaemonDBusInterface.forceOnNetworkChange()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Component {
            id: sectionHeading
            Rectangle {
                width: ListView.view.width
                height: childrenRect.height

                required property string section

                Text {
                    text: switch (parseInt(section)) {
                          case DevicesModel.Paired:
                              return  qsTr("Remembered");
                          case DevicesModel.Reachable:
                              return  qsTr("Available");
                          case (DevicesModel.Reachable | DevicesModel.Paired):
                              return  qsTr("Connected");
                          }

                    font.pixelSize: 14
                }
            }
        }

        //highlight
        Component { // 'real'  delegate
            id: highlightBar
            Rectangle {
                width: deviceList.currentItem == null? 0:deviceList.currentItem.width;
                height: deviceList.currentItem == null? 0:deviceList.currentItem.height
                color: "lightsteelblue"
                radius: 5
                y: deviceList.currentItem == null? 0:deviceList.currentItem.y
                Behavior on y {
                    SpringAnimation {
                        spring: 0
                        damping: 0
                    }
                }
            }
        }

        ListView {
            id: deviceList
            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            section.property: "status"
            section.delegate: sectionHeading

            model: DevicesSortProxyModel {
                sourceModel: DevicesModel {}
            }

            delegate: ItemDelegate
            {
                id: delegate
                height: 45
                width: ListView.view.width
                icon.name: iconName
                contentItem:Item {
                     Button{
                        id: img
                        flat: true
                        icon.name: iconName
                        anchors {
                            verticalCenter: parent.verticalCenter
                        }
                    }

                    Item{
                        anchors {
                            left: img.right
                            leftMargin: 3
                        }

                        Text {
                            id: titleText
                            text: name
                            font.pixelSize: 16
                        }
                        Text {
                            anchors {
                                top: titleText.bottom
                            }
                            text: toolTip
                            font.pixelSize: 10
                        }
                    }
                }

                highlighted: ListView.isCurrentItem

                onClicked: {
                    console.log("clicked")
                }

                MouseArea{
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onPositionChanged: deviceList.currentIndex = index
                    onExited: deviceList.currentIndex = -1
                }
            }

            highlight: highlightBar
        }

    }

    footer: ToolBar{
        height: 40
        contentItem: RowLayout {
            Label{
                text: announcedNameProperty.value
                Layout.fillWidth: true
            }

            DBusProperty {
                id: announcedNameProperty
                object: DaemonDBusInterface
                read: "announcedName"
                defaultValue: qsTr("DeviceName")
            }

            ToolButton{
                icon.name: "configure"
                ToolTip.text:  qsTr("Settings")
                ToolTip.visible: hovered
                onClicked: {
                    var windowComponent = Qt.createComponent(Qt.resolvedUrl("Settings.qml"))
                    var win = windowComponent.createObject(window)
                    windowComponent.destroy();
                    win.show()
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
