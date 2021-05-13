<!--
SPDX-FileCopyrightText: 2020 George Florea Bănuș <georgefb899@gmail.com>

SPDX-License-Identifier: CC-BY-4.0
-->

#### Donate: [GitHub Sponsors](https://github.com/sponsors/g-fb) | [Liberapay](https://liberapay.com/gfb/) | [PayPal](https://paypal.me/georgefloreabanus) | [Patreon](https://www.patreon.com/georgefb)

# Haruna Video Player

Haruna is an open source video player built with Qt/QML and libmpv.

![Haruna main window](./data/screenshots/haruna-dark.png)
[More Screenshots](./Screenshots.md)

# Install

https://flathub.org/apps/details/com.georgefb.haruna
### Stable
```
flatpak install flathub com.georgefb.haruna
flatpak run com.georgefb.haruna
```

### Beta
```
flatpak remote-add flathub-beta https://flathub.org/beta-repo/flathub-beta.flatpakrepo
flatpak install flathub-beta com.georgefb.haruna
flatpak run --branch=beta com.georgefb.haruna
```

[Flatpak setup guide](https://flatpak.org/setup/)

# AppImage

* Download appimage from [Releases](https://github.com/g-fb/haruna/releases)
* Make executable `chmod +x Haruna*.AppImage`
* Run
* Optional, use [AppImageLauncher](https://github.com/TheAssassin/AppImageLauncher)
* Note: On Fedora/Redhat users may need to install `libnsl` (`sudo dnf install libnsl`)

# Features

these are just some features that set Haruna apart from others players

- play online videos, through youtube-dl

- toggle playlist with mouse-over, playlist overlays the video

- auto skip chapter containing certain words

- configurable shortcuts and mouse buttons

- quick jump to next chapter by middle click on progress bar

# Dependencies

### Build time
- Qt5Core
- Qt5DBus
- Qt5Qml
- Qt5Quick
- Qt5QuickControls2
- Libmpv
- ExtraCmakeModules
- KF5Config
- KF5CoreAddons
- KF5FileMetaData
- KF5I18n
- KF5IconThemes
- KF5KIO
- KF5Kirigami2
- KF5XmlGui

### Runtime
- Kio-extras
- Breeze icons
- Breeze widgets style
- QQC2-Desktop-Style
- Youtube-dl

# Build

```
git clone https://github.com/g-fb/haruna
cd haruna && mkdir build && cd build
cmake ..
cmake --build .
./src/haruna
```
