#include "peony-application.h"
#include "menu-plugin-iface.h"

#include <QDebug>
#include <QDir>
#include <QPluginLoader>
#include <QString>
#include <QMenu>
#include <QTimer>

#include "preview-page-factory-manager.h"
#include "preview-page-plugin-iface.h"
#include "directory-view-factory-manager.h"
#include "directory-view-plugin-iface.h"

PeonyApplication::PeonyApplication(int &argc, char *argv[]) : QApplication (argc, argv)
{
    //check if first run
    //if not send message to server
    //else
    //  load plgin
    //  read from command line
    //  do with args
#ifdef MENU
    QDir pluginsDir(qApp->applicationDirPath());
    qDebug()<<pluginsDir;
    pluginsDir.cdUp();
    pluginsDir.cd("testdir");
    pluginsDir.setFilter(QDir::Files);

    qDebug()<<pluginsDir.entryList().count();
    Q_FOREACH(QString fileName, pluginsDir.entryList(QDir::Files)) {
        qDebug()<<fileName;
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        qDebug()<<pluginLoader.fileName();
        qDebug()<<pluginLoader.metaData();
        qDebug()<<pluginLoader.load();
        QObject *plugin = pluginLoader.instance();
        qDebug()<<"test start";
        if (plugin) {
            Peony::MenuPluginInterface *iface = qobject_cast<Peony::MenuPluginInterface *>(plugin);
            if (iface) {
                qDebug()<<iface->testPlugin()<<iface->name()<<iface->description();

                QWidget *widget = new QWidget;
                widget->setAttribute(Qt::WA_DeleteOnClose);
                QMenu *menu = new QMenu(widget);
                QStringList l;
                Peony::MenuPluginInterface::Types types = Peony::MenuPluginInterface::Types(Peony::MenuPluginInterface::File|
                                                                                            Peony::MenuPluginInterface::Volume|
                                                                                            Peony::MenuPluginInterface::DirectoryBackground|
                                                                                            Peony::MenuPluginInterface::DesktopBackground|
                                                                                            Peony::MenuPluginInterface::Other);
                auto fileActions = iface->menuActions(types, nullptr, l);
                for (auto action : fileActions) {
                    action->setParent(menu);
                }
                menu->addActions(fileActions);

                widget->setContextMenuPolicy(Qt::CustomContextMenu);
                widget->connect(widget, &QWidget::customContextMenuRequested, [menu](const QPoint /*&pos*/){
                    //menu->exec(pos);
                    menu->exec(QCursor::pos());
                });
                widget->show();
            }
        }
        qDebug()<<"testEnd";
    }
#endif

#define PREVIEW
#ifdef PREVIEW
    qDebug()<<"preview test";
    auto previewManager = Peony::PreviewPageFactoryManager::getInstance();
    qDebug()<<previewManager->getPluginNames();
    for (auto name : previewManager->getPluginNames()) {
        auto plugin = previewManager->getPlugin(name);
        auto w = plugin->createPreviewPage();
        w->startPreview();
        QTimer::singleShot(1000, [=](){
            w->cancel();
        });
        auto w1 = dynamic_cast<QWidget*>(w);
        w1->setAttribute(Qt::WA_DeleteOnClose);
        w1->show();
    }

#endif

#ifdef DIRECTORY_VIEW
    QDir pluginsDir(qApp->applicationDirPath());
    qDebug()<<pluginsDir;
    pluginsDir.cdUp();
    pluginsDir.cd("testdir2");
    pluginsDir.setFilter(QDir::Files);

    qDebug()<<"directory view test";
    auto directoryViewManager = Peony::DirectoryViewFactoryManager::getInstance();
    qDebug()<<directoryViewManager->getFactoryNames();
    auto names = directoryViewManager->getFactoryNames();
    for (auto name : names) {
        qDebug()<<name;
        auto factory = directoryViewManager->getFactory(name);
        auto view = factory->create();
        //BUG: it is not safe loading a new uri when
        //a directory is loading. enve thoug the file enumemeration
        //is cancelled and, the async method from GFileEnumerator
        //might still run in thread and return.
        //This cause program went crash.

        //view->setDirectoryUri("file:///");
        //view->beginLocationChange();
        //view->stopLocationChange();
        auto proxy = view->getProxy();
        qDebug()<<"2";
        proxy->setDirectoryUri("file:///", false);
        proxy->beginLocationChange();
        QTimer::singleShot(1000, [=](){
            proxy->invertSelections();
        });
        connect(proxy, &Peony::DirectoryViewProxyIface::viewDoubleClicked, [=](const QString &uri){
            qDebug()<<"double clicked"<<uri;
            proxy->setDirectoryUri(uri, false);
            proxy->beginLocationChange();
        });

        auto widget = dynamic_cast<QWidget*>(view);
        widget->setAttribute(Qt::WA_DeleteOnClose);
        widget->show();
    }
#endif
}
