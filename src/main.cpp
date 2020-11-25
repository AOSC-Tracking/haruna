/*
 * SPDX-FileCopyrightText: 2020 George Florea Bănuș <georgefb899@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "_debug.h"
#include "application.h"
#include "lockmanager.h"
#include "mpvobject.h"
#include "tracksmodel.h"
#include "subtitlesfoldersmodel.h"
#include "playlist/playlistitem.h"
#include "playlist/playlistmodel.h"
#include "worker.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickView>
#include <QThread>

#include <KAboutData>
#include <KI18n/KLocalizedString>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setOrganizationName("georgefb");
    QApplication::setOrganizationDomain("georgefb.com");
    QApplication::setWindowIcon(QIcon::fromTheme("com.georgefb.haruna"));

    QApplication app(argc, argv);

    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    QQuickStyle::setFallbackStyle(QStringLiteral("Fusion"));

    KAboutData aboutData(QStringLiteral("haruna"),
                         i18n("Haruna Video Player"),
                         QStringLiteral("0.2.2"));
    aboutData.setShortDescription(i18n("A simple video player."));
    aboutData.setLicense(KAboutLicense::GPL_V3);
    aboutData.setCopyrightStatement(i18n("(c) 2019"));
    aboutData.setOtherText(i18n("TO DO..."));
    aboutData.setHomepage(QStringLiteral("https://github.com/g-fb/haruna"));
    aboutData.setBugAddress(QStringLiteral("https://github.com/g-fb/haruna/issues").toUtf8());

    aboutData.addAuthor(i18n("George Florea Bănuș"),
                        i18n("Developer"),
                        QStringLiteral("georgefb899@gmail.com"),
                        QStringLiteral("https://georgefb.com"));

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addPositionalArgument(QStringLiteral("file"), i18n("File to open"));
    parser.process(app);
    aboutData.processCommandLine(&parser);

    // Qt sets the locale in the QGuiApplication constructor, but libmpv
    // requires the LC_NUMERIC category to be set to "C", so change it back.
    std::setlocale(LC_NUMERIC, "C");

    qmlRegisterType<MpvObject>("mpv", 1, 0, "MpvObject");
    qRegisterMetaType<QAction*>();
    qRegisterMetaType<TracksModel*>();
    qRegisterMetaType<KFileMetaData::PropertyMap>("KFileMetaData::PropertyMap");

    std::unique_ptr<Application> myApp = std::make_unique<Application>();
    std::unique_ptr<LockManager> lockManager = std::make_unique<LockManager>();
    std::unique_ptr<SubtitlesFoldersModel> subsFoldersModel = std::make_unique<SubtitlesFoldersModel>();

    for (auto i = 0; i < parser.positionalArguments().size(); ++i) {
        myApp->addArgument(i, parser.positionalArguments().at(i));
    }

    auto worker = Worker::instance();
    auto thread = new QThread();
    worker->moveToThread(thread);
    QObject::connect(thread, &QThread::finished,
                     worker, &Worker::deleteLater);
    QObject::connect(thread, &QThread::finished,
                     thread, &QThread::deleteLater);
    thread->start();

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);

    PlayListModel playListModel;
    engine.rootContext()->setContextProperty("playListModel", &playListModel);
    qmlRegisterUncreatableType<PlayListModel>("PlayListModel", 1, 0, "PlayListModel",
                                               QStringLiteral("PlayListModel should not be created in QML"));

    engine.rootContext()->setContextProperty(QStringLiteral("app"), myApp.get());
    qmlRegisterUncreatableType<Application>("Application", 1, 0, "Application",
                                            QStringLiteral("Application should not be created in QML"));

    engine.rootContext()->setContextProperty(QStringLiteral("lockManager"), lockManager.release());
    qmlRegisterUncreatableType<LockManager>("LockManager", 1, 0, "LockManager",
                                            QStringLiteral("LockManager should not be created in QML"));

    engine.rootContext()->setContextProperty(QStringLiteral("subsFoldersModel"), subsFoldersModel.release());

    myApp.get()->setupQmlSettingsTypes();

    engine.load(url);
    return QApplication::exec();
}

