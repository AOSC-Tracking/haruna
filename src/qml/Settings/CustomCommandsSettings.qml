/*
 * SPDX-FileCopyrightText: 2021 George Florea Bănuș <georgefb899@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import org.kde.kirigami 2.11 as Kirigami
import org.kde.haruna 1.0
import Haruna.Components 1.0

SettingsBasePage {
    id: root

    hasHelp: true
    helpFile: ":/CustomCommandsSettings.html"
    docPage: "help:/haruna/CustomCommandsSettings.html"

    ColumnLayout {
        id: centerLayout

        visible: customCommandsView.count === 0
        anchors.centerIn: parent

        Label {
            text: i18n("No custom commands yet")
            Layout.alignment: Qt.AlignCenter
        }

        Button {
            text: i18n("&Add command")
            onClicked: applicationWindow().pageStack.replace("qrc:/EditCustomCommand.qml")
            Layout.alignment: Qt.AlignCenter
        }
    }

    Component {
        id: customCommandDelegate

        Kirigami.AbstractListItem {
            id: customCommandItem

            height: Kirigami.Units.gridUnit * 3
            padding: 0

            contentItem: RowLayout {
                anchors.fill: parent
                spacing: 0

                Kirigami.ListItemDragHandle {
                    listItem: customCommandItem
                    listView: customCommandsView
                    onMoveRequested: customCommandsModel.moveRows(oldIndex, newIndex)
                }

                CheckBox {
                    visible: model.type === "startup"
                    checked: model.loadOnStartup
                    onCheckStateChanged: {
                        customCommandsModel.toggleCustomCommand(model.commandId, model.index, checked)
                    }

                    ToolTip {
                        text: i18n("Don't set on next startup")
                        delay: 0
                    }
                }

                Kirigami.Icon {
                    source: model.type === "shortcut" ? "configure-shortcuts" : "code-context"
                    width: Kirigami.Units.iconSizes.small
                    height: Kirigami.Units.iconSizes.small
                    enabled: model.loadOnStartup
                }

                LabelWithTooltip {
                    text: model.command
                    elide: Text.ElideRight
                    enabled: model.loadOnStartup

                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    Layout.fillWidth: true
                    Layout.margins: Kirigami.Units.largeSpacing
                }

                ToolButton {
                    text: i18n("Shortcut")
                    icon.name: "configure-shortcuts"
                    visible: model.type === "shortcut"
                    onClicked: actionsManager.configureShortcuts(model.command)

                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                }

                ToolButton {
                    icon.name: "edit-entry"
                    enabled: model.loadOnStartup
                    Layout.rightMargin: Kirigami.Units.largeSpacing
                    onClicked: applicationWindow().pageStack.replace("qrc:/EditCustomCommand.qml",
                                                                     {
                                                                         command: model.command,
                                                                         osdMessage: model.osdMessage,
                                                                         type: model.type,
                                                                         commandId: model.commandId,
                                                                         index: model.index,
                                                                         mode: EditCustomCommand.Mode.Edit
                                                                     })
                }
            }
        }
    }

    ListView {
        id: customCommandsView

        model: customCommandsModel
        delegate: Kirigami.DelegateRecycler {
            width: customCommandsView.width
            sourceComponent: customCommandDelegate
        }
    }

    footer: ToolBar {
        visible: customCommandsView.count > 0

        RowLayout {
            anchors.fill: parent

            ToolButton {
                id: addButton

                text: i18n("&Add")
                icon.name: "list-add"
                onClicked: applicationWindow().pageStack.replace("qrc:/EditCustomCommand.qml")
                Layout.alignment: Qt.AlignRight
            }
        }
    }

}
