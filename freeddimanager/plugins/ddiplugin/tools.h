/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2015 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
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
 *  Main Developer: Eric MAEKER, MD <eric.maeker@gmail.com>                *
 *  Contributors:                                                          *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
#ifndef DRUGSDB_TOOLS_H
#define DRUGSDB_TOOLS_H

#include <drugsdb/drugsdb_exporter.h>

#include <QString>
#include <QHash>
#include <QMultiHash>
#include <QVector>
#include <QVariant>
QT_BEGIN_NAMESPACE
class QProgressDialog;
class QSqlDatabase;
QT_END_NAMESPACE

/**
 * \file tools.h
 * \author Eric MAEKER
 * \version 0.8.0
 * \date 21 Oct 2012
*/

namespace DrugsDB {
class DrugDatabaseDescription;
class Drug;
namespace Internal {
class DrugBaseEssentials;
}  // namespace Internal
}  // namespace DrugsDB

namespace DrugsDB {
namespace Tools {

DRUGSDB_EXPORT QString noAccent(const QString & s);

DRUGSDB_EXPORT QString getBlock(const QString &content, const int posStart, int &posEnd, const QRegExp &delimiter);
DRUGSDB_EXPORT QString getBlock(const QString &content, const int posStart, int &posEnd, const QString &delimiter);

DRUGSDB_EXPORT bool executeProcess(const QString &proc);
DRUGSDB_EXPORT bool executeSqlFile(const QString &connectionName, const QString &fileName, QProgressDialog *dlg = 0);
DRUGSDB_EXPORT bool executeSqlQuery(const QString &sql, const QString &dbName, const QString &file = QString::null, int line = -1);

DRUGSDB_EXPORT QString databaseOutputPath();
DRUGSDB_EXPORT bool connectDatabase(const QString &connection, const QString &fileName);
DRUGSDB_EXPORT bool signDatabase(const QString &connectionName);

DRUGSDB_EXPORT int addLabels(DrugsDB::Internal::DrugBaseEssentials *database, const int masterLid, const QMultiHash<QString, QVariant> &trLabels);
DRUGSDB_EXPORT bool createAtc(DrugsDB::Internal::DrugBaseEssentials *database, const QString &code, const QMultiHash<QString, QVariant> &trLabels, const int forceAtcId = -1, const bool warnDuplicates = true);
DRUGSDB_EXPORT bool addInteraction(DrugsDB::Internal::DrugBaseEssentials *database, const QStringList &atc1, const QStringList &atc2, const QString &type, const QMultiHash<QString, QVariant> &risk, const QMultiHash<QString, QVariant> &management);
DRUGSDB_EXPORT int addBibliography(DrugsDB::Internal::DrugBaseEssentials *database, const QString &type, const QString &link, const QString &reference, const QString &abstract, const QString &explain = QString::null, const QString &xml = QString::null);

DRUGSDB_EXPORT QVector<int> getAtcIdsFromLabel(DrugsDB::Internal::DrugBaseEssentials *database, const QString &label);
DRUGSDB_EXPORT QVector<int> getAtcIdsFromCode(DrugsDB::Internal::DrugBaseEssentials *database, const QString &code);

}  // End namespace Tools
}  // End namespace DrugsDB

#endif // DRUGSDB_TOOLS_H
