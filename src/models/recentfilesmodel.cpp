/*
 * SPDX-FileCopyrightText: 2021 George Florea Bănuș <georgefb899@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "recentfilesmodel.h"

#include "generalsettings.h"
#include <global.h>

#include <QDebug>
#include <QMenu>

#include <KRecentFilesAction>
#include <KSharedConfig>

RecentFilesModel::RecentFilesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    auto config = KSharedConfig::openConfig(Global::instance()->appConfigFilePath());
    m_recentFilesConfigGroup = config->group(QStringLiteral("RecentFiles"));
    m_recentFilesAction = new KRecentFilesAction(this);
    m_recentFilesAction->setMaxItems(GeneralSettings::maxRecentFiles());
    m_recentFilesAction->loadEntries(m_recentFilesConfigGroup);

    populate();
}

int RecentFilesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_urls.count();
}

QVariant RecentFilesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    QUrl url = m_urls.at(index.row());

    switch (role) {
    case PathRole:
        return QVariant(url.path());
    case NameRole:
        return QVariant(url.fileName());
    }

    return QVariant();
}

QHash<int, QByteArray> RecentFilesModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[PathRole] = "path";
    roles[NameRole] = "name";

    return roles;
}

void RecentFilesModel::populate()
{
    beginResetModel();
    m_urls.clear();
    endResetModel();

    m_recentFilesAction->setMaxItems(GeneralSettings::maxRecentFiles());
    for (int i = 0; i < m_recentFilesAction->maxItems(); i++) {
        auto file = m_recentFilesConfigGroup.readPathEntry(QStringLiteral("File%1").arg(i + 1), QString());
        if (file.isEmpty()) {
            break;
        }
        beginInsertRows(QModelIndex(), 0, 0);
        m_urls.prepend(file);
        endInsertRows();
    }
}

void RecentFilesModel::addUrl(const QString &path)
{
    auto config = KSharedConfig::openConfig(Global::instance()->appConfigFilePath());
    QUrl url(path);
    if (!url.isLocalFile() && url.scheme().isEmpty()) {
        url.setScheme("file");
    }

    m_recentFilesAction->addUrl(url, url.fileName());
    m_recentFilesAction->saveEntries(m_recentFilesConfigGroup);
    m_recentFilesConfigGroup.sync();

    auto exists = [url](const QUrl &_url) {
        return _url.path() == url.path();
    };
    beginResetModel();
    auto result = std::find_if(m_urls.begin(), m_urls.end(), exists);
    if (result != m_urls.end()) {
        m_urls.removeAll(*result);
    }
    m_urls.prepend(url);
    endResetModel();
}

void RecentFilesModel::clear()
{
    beginResetModel();
    for (const auto &url : m_recentFilesAction->urls()) {
        m_recentFilesAction->removeUrl(url);
    }
    m_recentFilesAction->saveEntries(m_recentFilesConfigGroup);
    m_recentFilesConfigGroup.sync();
    m_urls.clear();
    endResetModel();
}
