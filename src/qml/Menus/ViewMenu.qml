/*
 * SPDX-FileCopyrightText: 2020 George Florea Bănuș <georgefb899@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12

Menu {
    id: root

    title: i18n("&View")

    MenuItem { action: actions.toggleFullscreenAction }

    MenuItem {
        action: actions.toggleMenuBarAction
        text: menuBar.visible ? i18n("Hide Menubar") : i18n("Show Menubar")
    }
    MenuItem {
        action: actions.toggleHeaderAction
        text: header.visible ? i18n("Hide Toolbar") : i18n("Show Toolbar")
    }
}
