/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2011 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program (COPYING.FREEMEDFORMS file).                   *
 *  If not, see <http://www.gnu.org/licenses/>.                            *
 ***************************************************************************/
/***************************************************************************
 *   Main Developpers :                                                    *
 *       Eric MAEKER, MD <eric.maeker@gmail.com>                           *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#include "servermanager.h"
#include "core.h"
#include "widgets/installpackdialog.h"
#include "serverengines/localserverengine.h"
#include "serverengines/httpserverengine.h"

#include <utils/log.h>
#include <utils/global.h>
#include <translationutils/constants.h>
#include <translationutils/trans_filepathxml.h>

#include <quazip/global.h>

#include <QDir>
#include <QDomDocument>
#include <QDomElement>

#include <QDebug>

using namespace DataPack;
using namespace DataPack::Internal;
using namespace Trans::ConstantTranslations;

namespace {

const char * const TAG_ROOT                 = "ServerManagerConfig";
const char * const TAG_SERVER               = "Server";
const char * const TAG_PACK                 = "Pack";

// Server attribs
const char * const ATTRIB_URL                 = "url";
const char * const ATTRIB_LASTCHECK           = "lastChk";
const char * const ATTRIB_RECORDEDVERSION     = "recVer";
const char * const ATTRIB_USERUPDATEFREQUENCY = "uUpFq";

// Pack specific attribs
const char * const ATTRIB_INSTALLED         = "inst";
const char * const ATTRIB_INSTALLPATH       = "instPath";

const char * const SERVER_CONFIG_FILENAME   = "server.conf.xml";

}  // End namespace Anonymous

static inline DataPack::Core &core() {return DataPack::Core::instance();}

ServerManager::ServerManager(QObject *parent) :
    IServerManager(parent)
{
    setObjectName("ServerManager");

    // Create engines
    m_LocalEngine = new LocalServerEngine(this);
    m_HttpEngine = new HttpServerEngine(this);
    m_WorkingEngines << m_LocalEngine << m_HttpEngine;
}

ServerManager::~ServerManager()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Config and path //////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
bool ServerManager::setGlobalConfiguration(const QString &xmlContent, QString *errorMsg)
{
    QDomDocument doc;
    QString msg;
    int col, line;
    if (!doc.setContent(xmlContent, &msg, &line, &col)) {
        if (errorMsg) {
            errorMsg->append(QString("XML Error (l:%1;c:%2): %3.").arg(line).arg(col).arg(msg));
        }
        return false;
    }

    QDomElement root = doc.firstChildElement(::TAG_ROOT);
    // Read servers
    QDomElement server = root.firstChildElement(::TAG_SERVER);
    while (!server.isNull()) {
        Server s;
        s.fromSerializedString(server.attribute(::ATTRIB_URL));
        s.setLastChecked(QDateTime::fromString(server.attribute(::ATTRIB_LASTCHECK), Qt::ISODate));
        s.setLocalVersion(server.attribute(::ATTRIB_RECORDEDVERSION));
        s.setUserUpdateFrequency(server.attribute(::ATTRIB_USERUPDATEFREQUENCY).toInt());
        m_Servers.append(s);
        server = server.nextSiblingElement(::TAG_SERVER);
    }
    // Read packs
//    QDomElement pack = root.firstChildElement(::TAG_PACK);
//    while (!server.isNull()) {
//        Pack p;
//        /** \todo here */
//        pack = pack.nextSiblingElement(::TAG_PACK);
//    }
    return true;
}

QString ServerManager::xmlConfiguration() const
{
    QDomDocument doc;
    QDomElement root = doc.createElement(::TAG_ROOT);
    doc.appendChild(root);
    for(int i = 0; i < m_Servers.count(); ++i) {
        const Server &s = m_Servers.at(i);
        QDomElement e = doc.createElement(::TAG_SERVER);
        root.appendChild(e);
        e.setAttribute(::ATTRIB_URL, s.serialize());
        e.setAttribute(::ATTRIB_RECORDEDVERSION, s.localVersion());
        e.setAttribute(::ATTRIB_LASTCHECK, s.lastChecked().toString(Qt::ISODate));
        e.setAttribute(::ATTRIB_USERUPDATEFREQUENCY, s.userUpdateFrequency());
    }
    return doc.toString(2);
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Server list /////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
bool ServerManager::addServer(const QString &url)
{
    Server server(url);
    return addServer(server);
}

bool ServerManager::addServer(const Server &server)
{
    // check if a server already exists with the same URL
    foreach (Server child, m_Servers) {
        if (child == server)
            return false;
    }
    m_Servers.append(server);
    Q_EMIT serverAdded(m_Servers.count() - 1);
    return true;
}

int ServerManager::serverCount() const
{
    return m_Servers.count();
}

Server ServerManager::getServerAt(int index) const
{
    if (index < m_Servers.count() && index >= 0)
        return m_Servers.at(index);
    return Server();
}

int ServerManager::getServerIndex(const QString &url) const
{
    for (int i = 0; i < m_Servers.count(); i++)
        if (m_Servers.at(i).url() == url)
            return i;
    return -1;
}

void ServerManager::removeServerAt(int index)
{
    if (index >= 0 && index < m_Servers.count()) {
        Server removed = m_Servers.at(index);
        Q_EMIT serverAboutToBeRemoved(removed);
        Q_EMIT serverAboutToBeRemoved(index);
        m_Servers.remove(index);
        Q_EMIT serverRemoved(removed);
        Q_EMIT serverRemoved(index);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Updates and installs ////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void ServerManager::getAllDescriptionFile()
{
    for(int i=0; i < m_Servers.count(); ++i) {
        Server *s = &m_Servers[i];
        qWarning() << "getAllDescription" << i << s->nativeUrl();
        for(int j = 0; j < m_WorkingEngines.count(); ++j) {
            IServerEngine *engine = m_WorkingEngines.at(j);
            if (engine->managesServer(*s)) {
                ServerEngineQuery query;
                query.server = s;
                query.forceDescriptionFromLocalCache = false;
                query.downloadDescriptionFiles = true;
                query.downloadPackFile = false;
                engine->addToDownloadQueue(query);
            }
        }
    }
    for(int j = 0; j < m_WorkingEngines.count(); ++j) {
        IServerEngine *engine = m_WorkingEngines.at(j);
        if (engine->downloadQueueCount() > 0) {
            connect(engine, SIGNAL(queueDowloaded()), this, SLOT(engineDescriptionDownloadDone()));
            engine->startDownloadQueue();
        }
    }
}

void ServerManager::checkServerUpdates()
{
    WARN_FUNC << m_Servers.count();
    // Get installed packs uuid && version

    // Compare installed pack versions with server description


//    for(int i=0; i < m_Servers.count(); ++i) {
//        Server &s = m_Servers[i];
//        qDebug("%d: %s", i, qPrintable(s.url()));
//        if (s.isLocalServer()) {
//            // check directly
//            s.fromXml(Utils::readTextFile(s.url(Server::ServerConfigurationFile), Utils::DontWarnUser));
//            // move a copy of the description in the working path of server manager
//        } else {
//            // FTP | HTTP
//            // Download server.conf.xml
//            QNetworkRequest request = createRequest(s.url(Server::ServerConfigurationFile));
//            QNetworkReply *reply = m_NetworkAccessManager->get(request);
//            m_replyToData.insert(reply, ReplyData(reply, &s, Server::ServerConfigurationFile));
//            connect(reply, SIGNAL(readyRead()), this, SLOT(serverReadyRead()));
//            connect(reply, SIGNAL(finished()), this, SLOT(serverFinished()));
//                        // TODO manage errors
//        }
//    }
//    // TODO THIS LINE IS ONLY FOR TESTING PURPOSE
//    checkServerUpdatesAfterDownload();
}

void ServerManager::engineDescriptionDownloadDone()
{
    // if all engines download done -> emit signal
    bool __emit = true;
    for(int i = 0; i < m_WorkingEngines.count(); ++i) {
        if (m_WorkingEngines.at(i)->downloadQueueCount()>0) {
            /** \todo disconnect engine signal */
            qWarning() << m_WorkingEngines.at(i)->objectName() << m_WorkingEngines.at(i)->downloadQueueCount();
            __emit = false;
        }
    }
    if (__emit)
        Q_EMIT allServerDescriptionAvailable();
}

void ServerManager::registerPack(const Server &server, const Pack &pack)
{
    m_Packs.insertMulti(server.uuid(), pack);
}



/////////////////////////////////////////////////////////////////////////////////////////

void ServerManager::connectServer(const Server &server, const ServerIdentification &ident)
{
    // TODO
    Q_UNUSED(server);
    Q_UNUSED(ident);
}

ServerDescription ServerManager::downloadServerDescription(const Server &server)
{
    // TODO
    Q_UNUSED(server);
    ServerDescription desc;
    return desc;
}

QList<PackDescription> ServerManager::downloadPackDescription(const Server &server, const Pack &pack)
{
    // TODO
    Q_UNUSED(server);
    Q_UNUSED(pack);
    QList<PackDescription> list;
    return list;
}

Pack ServerManager::downloadAndUnzipPack(const Server &server, const Pack &pack)
{
    // TODO
    Q_UNUSED(server);
    Q_UNUSED(pack);
    return pack;
}

bool ServerManager::downloadDataPack(const Server &server, const Pack &pack, QProgressBar *progressBar)
{
    Q_UNUSED(server);
    Q_UNUSED(pack);
    Q_UNUSED(progressBar);
    Q_ASSERT(progressBar);
    // TODO pour guillaume
    // Juste télécharger rien de plus dans le rép m_InstallPath
//    QString url = server.url(Server::PackFile, pack.serverFileName());
    return true;
}

void ServerManager::checkAndInstallPack(const Server &server, const Pack &pack, QProgressBar *progressBar)
{
    Q_UNUSED(server);
    Q_UNUSED(pack);
    Q_UNUSED(progressBar);
    Q_ASSERT(progressBar);
    disconnect(this, SIGNAL(packDownloaded(Server,Pack,QProgressBar*)), this, SLOT(checkAndInstallPack(Server,Pack,QProgressBar*)));
}

void ServerManager::checkInstalledPacks()
{
    if (!m_InstalledPacks.isEmpty())
        return;
    // Scan the install dir for packconfig.xml files and read them
    foreach(const QFileInfo &info, Utils::getFiles(QDir(core().installPath()), "packconfig.xml")) {
        Pack p;
        p.fromXmlFile(info.absoluteFilePath());
        m_InstalledPacks.append(p);
    }
}

bool ServerManager::isDataPackInstalled(const Pack &pack)
{
    return isDataPackInstalled(pack.uuid(), pack.version());
}

bool ServerManager::isDataPackInstalled(const QString &packUid, const QString &packVersion)
{
    Q_UNUSED(packUid);
    Q_UNUSED(packVersion);
    checkInstalledPacks();
    foreach(const Pack &p, m_InstalledPacks) {
        if (p.uuid()==packUid && p.version()==packVersion) {
            return true;
        }
    }

    return false;
}

bool ServerManager::installDataPack(const Pack &pack, QProgressBar *progressBar)
{
    Q_ASSERT(!core().persistentCachePath().isEmpty());
    Q_ASSERT(!core().installPath().isEmpty());
    m_PacksToInstall.clear();

    // Dialog checks packs (dependencies...)
    InstallPackDialog dlg;
    dlg.setPackToInstall(pack);
    if (dlg.exec()==QDialog::Rejected) {
        return false;
    }

    // TODO: Create a new view to show progressBars and infos

    // Get the pack list to install
    m_PacksToInstall = dlg.packsToInstall();
    LOG(QString("Pack installation requested. %1 pack(s) to install").arg(m_PacksToInstall.count()));

    // Download all the packs
    for(int i = 0; i<m_PacksToInstall.count(); ++i) {
        // get the server
        Pack &pack = m_PacksToInstall[i];
        Server server = getServerForPack(pack);
//        qWarning() << "TESTING Pack" << pack.uuid() << "Server found" << server.uuid();
        if (!server.isNull()) {
            // Pack already downloaded ?
            QFileInfo info(core().persistentCachePath() + QDir::separator() + pack.uuid() + QDir::separator() + QFileInfo(pack.serverFileName()).fileName());
//            qWarning() << "   already downloaded ?" << info.exists();
            if (info.exists()) {
                // Test local version of the pack
                Pack cached;
                cached.fromXmlFile(core().persistentCachePath() + QDir::separator() + pack.uuid() + QDir::separator() + "packconfig.xml");
                if (cached.version() == pack.version()) {
//                    qWarning() << "   pack already downloaded" << pack.uuid() << pack.description().data(PackDescription::Version);
                    continue;
                } else {
//                    qWarning() << "    Wrong version" << cached.version() << pack.version();
                    // delete local data
                    QFile::remove(info.absoluteFilePath());
                    QFile::remove(core().persistentCachePath() + QDir::separator() + pack.uuid() + QDir::separator() + "packconfig.xml");
                }
            }

            // Download the pack from this server
            for(int j=0; j<m_WorkingEngines.count(); ++j) {
                if (m_WorkingEngines.at(j)->managesServer(server)) {
                    ServerEngineQuery query;
                    query.downloadPackFile = true;
                    query.pack = &pack;
                    query.server = &server;
                    m_WorkingEngines.at(j)->addToDownloadQueue(query);
                    LOG(QString("Adding %1 to %2 download queue").arg(pack.uuid()).arg(server.uuid()));
                }
            }
        }
    }
    // Start downloads
    bool downloading = false;
    for(int i=0; i<m_WorkingEngines.count(); ++i) {
        if (m_WorkingEngines.at(i)->downloadQueueCount()>0) {
            downloading = true;
            connect(m_WorkingEngines.at(i), SIGNAL(queueDowloaded()), this, SLOT(packDownloadDone()));
            m_WorkingEngines.at(i)->startDownloadQueue();
        }
    }
    if (!downloading)
        packDownloadDone();
    return true;
}

void ServerManager::packDownloadDone()
{
    Q_ASSERT(!core().persistentCachePath().isEmpty());
    Q_ASSERT(!core().installPath().isEmpty());
    // All engine finished ?
    for(int i=0; i<m_WorkingEngines.count(); ++i) {
        if (m_WorkingEngines.at(i)->downloadQueueCount()>0) {
//            qWarning() << m_WorkingEngines.at(i) << "unfinished download";
            return;
        }
    }

    // Some log for debugging purpose
    LOG(QString("Requested packs are downloaded in %1").arg(core().persistentCachePath()));
    QStringList logPacks;
    for(int i=0; i < m_PacksToInstall.count(); ++i) {
        const Pack &p = m_PacksToInstall.at(i);
        logPacks << QString("%1 (%2, %3)")
                    .arg(p.description().data(PackDescription::Label).toString())
                    .arg(p.uuid())
                    .arg(p.version());
    }
    LOG(QString("Requested packs: %1").arg(logPacks.join("; ")));

    // Copy/Unzip all packs to datapackInstallPath according to PackDescription::InstallToPath
    for(int i=0; i < m_PacksToInstall.count(); ++i) {
        const Pack &p = m_PacksToInstall.at(i);
        const QString pathTo = p.unzipPackToPath();
        QDir to(pathTo);
        if (!to.exists()) {
            to.mkpath(pathTo);
        }
        // Unzip pack to the install path
        if (!QuaZipTools::unzipFile(p.persistentlyCachedZipFileName(), pathTo))
            LOG_ERROR(QString("Unable to unzip pack file %1 to %2").arg(p.persistentlyCachedZipFileName()).arg(pathTo));
        // Add the pack description for future analysis (update, remove...)
        qWarning() << "INSTALL XML" <<p.persistentlyCachedXmlConfigFileName() << p.installedXmlConfigFileName();
        QFile::copy(p.persistentlyCachedXmlConfigFileName(), p.installedXmlConfigFileName());
    }
    m_PacksToInstall.clear();
    m_InstalledPacks.clear();
    checkInstalledPacks();
}

bool ServerManager::removeDataPack(const Pack &pack, QProgressBar *progressBar)
{
    // TODO
    Q_UNUSED(progressBar);
    // check if pack is installed
    // if installed: remove install path && persistentCache path
    m_InstalledPacks.clear();
    checkInstalledPacks();
    return false;
}

bool ServerManager::updateDataPack(const Pack &pack, QProgressBar *progressBar)
{
    // TODO
    Q_UNUSED(progressBar);
    Q_UNUSED(pack);
    m_InstalledPacks.clear();
    checkInstalledPacks();
    return false;
}

QList<Pack> ServerManager::packDependencies(const Pack &pack, const PackDependencyData::TypeOfDependence &dependence)
{
    QList<Pack> toReturn;
    for(int i = 0; i < pack.dependencies().count(); ++i) {
        if (pack.dependencies().at(i).type()!=dependence)
            continue;
        const QString &uid = pack.dependencies().at(i).uuid();
        const QString &version = pack.dependencies().at(i).version();
//        if (!isDataPackInstalled(uid, version)) {
            // Do we have this pack in our servers ?
            for(int j=0; j < m_Servers.count(); ++j) {
                const QList<Pack> &packs = getPackForServer(m_Servers.at(i));
                for(int z=0; z < packs.count(); ++z) {
                    if (packs.at(z).uuid().compare(uid,Qt::CaseInsensitive)==0 &&
                            packs.at(z).version().compare(version,Qt::CaseInsensitive)==0) {
                        toReturn << packs.at(z);
                    }
                }
            }
//        }
    }
    return toReturn;
}

void ServerManager::connectAndUpdate(int index)
{
    Q_UNUSED(index);
//    if (index < m_Servers.count() && index >= 0)
//        m_Servers.at(index).connectAndUpdate();
}

QList<PackDescription> ServerManager::getPackDescription(const Server &server)
{
    WARN_FUNC;
    // If Pack list already known return it
    QList<PackDescription> toReturn;
    const QStringList keys = m_Packs.uniqueKeys();
    if (keys.contains(server.uuid(), Qt::CaseInsensitive)) {
        QList<Pack> packs = m_Packs.values(server.uuid());
        for(int i = 0; i < packs.count(); ++i) {
            toReturn << packs.at(i).description();
        }
        return toReturn;
    }

    createServerPackList(server);

    QList<Pack> packs = m_Packs.values(server.url());
    for(int i = 0; i < packs.count(); ++i) {
        toReturn << packs.at(i).description();
    }
    return toReturn;
}

QList<Pack> ServerManager::getPackForServer(const Server &server)
{
    createServerPackList(server);
    return m_Packs.values(server.uuid());
}

Server ServerManager::getServerForPack(const Pack &pack)
{
    for(int i=0; i<m_Servers.count();++i) {
        createServerPackList(m_Servers.at(i));
        const QString &uuid = m_Servers.at(i).uuid();
        if (m_Packs.values(uuid).contains(pack)) {
            return m_Servers.at(i);
        }
    }
    return Server();
}

void ServerManager::createServerPackList(const Server &server)
{
    if (!m_Packs.values(server.uuid()).isEmpty()) {
        return;
    }
    // Get the server config
    foreach(const QString &file, server.content().packDescriptionFileNames()) {
        QString path = server.url();
        path = path.replace("file:/", "") + QDir::separator() + file;
        QFileInfo f(path); // relative path
        Pack pack;
        // Read the packDescription
        pack.fromXmlFile(f.absoluteFilePath());
        // Store in the cache
        m_Packs.insertMulti(server.uuid(), pack);
    }
}

void ServerManager::checkServerUpdatesAfterDownload()
{
    for(int i=0; i < m_Servers.count(); ++i) {
        Server &s = m_Servers[i];
        if (s.updateState() == Server::UpdateAvailable) {
            qWarning() << "UPDATE" << s.url() << s.localVersion() << s.description().data(ServerDescription::Version).toString();
        }
        s.setLastChecked(QDateTime::currentDateTime());
//        if (s.isConnected())
//           Q_EMIT serverConnected(s, ServerIdentification());
//        s.setLocalVersion();
    }
    Q_EMIT serverUpdateChecked();
}

