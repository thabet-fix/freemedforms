/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2011 by Eric MAEKER, MD (France) <eric.maeker@free.fr>        *
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
#include "global.h"

#include <translationutils/constanttranslations.h>
#include <utils/log.h>

#include <QApplication>
#include <QWidget>
#include <QDesktopWidget>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QLineEdit>
#include <QTextCodec>
#include <QByteArray>
#include <QFont>
#include <QFileDialog>
#include <QDomNode>
#include <QDomDocument>
#include <QInputDialog>
#include <QProcess>
#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QTextDocument>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QAbstractButton>
#include <QTextDocument>
#include <QCryptographicHash>

/**
  \namespace Utils
  \brief Some global funtions and classes for the applications.
*/

using namespace Utils;
using namespace Trans::ConstantTranslations;

namespace Utils {

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////   LIB AND OS FUNCTIONS   ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool isDebugCompilation()
{
#ifdef DEBUG
    return true;
#else
    return false;
#endif
}
bool isSvnBuild()
{
#ifdef FULLAPPLICATION_BUILD
    return false;
#else
    return true;
#endif
}
bool isFullApplication()
{
#ifdef FULLAPPLICATION_BUILD
    return true;
#else
    return false;
#endif
}
bool isRunningOnMac()
{
#ifdef Q_OS_MAC
    return true;
#else
    return false;
#endif
}
bool isRunningOnWin()
{
#ifdef Q_OS_WIN
    return true;
#else
    return false;
#endif
}
bool isRunningOnLinux()
{
#ifdef Q_OS_LINUX
    return true;
#else
    return false;
#endif
}
bool isRunningOnFreebsd()
{
#ifdef Q_OS_FREEBSD
    return true;
#else
    return false;
#endif
}
QString uname()
{
    QString system;
    if (isRunningOnMac())
        system = "MacOs";
    else if (isRunningOnLinux())
        system = "Linux";
    else if (isRunningOnFreebsd())
        system = "FreeBSD";
    if (system.isEmpty())
        return QString();
    QProcess uname;
    uname.start("uname", QStringList() << "-a");
    if (!uname.waitForStarted())
        Utils::Log::addError("Utils", QApplication::translate("Utils", "Error while retrieve informations of uname under %1").arg(system) ,
                             __FILE__, __LINE__);
    if (!uname.waitForFinished())
        Utils::Log::addError("Utils", QApplication::translate("Utils", "Error while retrieve informations of uname under %1").arg(system) ,
                             __FILE__, __LINE__);
    return uname.readAll();
}
QString osName()
{
    if (isRunningOnLinux())
        return "Linux";
    else if (isRunningOnMac())
        return "MacOs";
    else if (isRunningOnWin())
        return "Windows";
    else if (isRunningOnFreebsd())
        return "FreeBSD";
    return QString();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////   FILES FUNCTIONS   /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief This function deletes all files in the selected dir (non recursively) and tries to remove the dir */
bool removeDir(const QString &name, QString *error)
{
    error->clear();
    QDir dir(name);
    if (!dir.exists()) {
        error->append(Trans::ConstantTranslations::tkTr(Trans::Constants::PATH_1_DOESNOT_EXISTS).arg(name));
        return false;
    }
    // is there is directory inside ? --> return false
    QStringList list = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    if (list.count()) {
        error->append(Trans::ConstantTranslations::tkTr(Trans::Constants::PATH_1_CONTAINS_DIRS).arg(name));
        return false;
    }
    // delete files
    list = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    foreach(const QString &f, list) {
        if (!dir.remove(f)) {
            error->append(Trans::ConstantTranslations::tkTr(Trans::Constants::FILE_1_CANNOT_BE_REMOVED).arg(f));
            return false;
        }
    }
    // delete dir
    if (!dir.rmpath(dir.absolutePath())) {
        error->append(Trans::ConstantTranslations::tkTr(Trans::Constants::PATH_1_CANNOT_BE_REMOVED).arg(dir.absolutePath()));
        return false;
    }
    return true;
}

QFileInfoList getFiles(QDir fromDir, const QStringList &filters, bool recursive)
{
    QFileInfoList files;
    foreach (const QFileInfo & file, fromDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::Name)) {
        if (file.isFile() && (filters.isEmpty() || QDir::match(filters, file.fileName())))
            files << file;
        else if (file.isDir() && recursive) {
            fromDir.cd(file.filePath());
            files << getFiles(fromDir, filters);
            fromDir.cdUp();
        }
    }
    return files;
}

QFileInfoList getFiles(QDir fromDir, const QString &filter, bool recursive)
{
    return getFiles(fromDir, filter.isEmpty() ? QStringList() : QStringList(filter), recursive);
}

QFileInfoList getDirs(QDir fromDir, const QStringList &filters, bool recursive)
{
    QFileInfoList dirs;
    foreach (const QFileInfo &file, fromDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::IgnoreCase)) {
        if (file.isFile() && (filters.isEmpty() || QDir::match(filters, file.fileName())))
            dirs << file;
        else if (file.isDir() && recursive) {
            fromDir.cd(file.filePath());
            dirs << getFiles(fromDir, filters);
            fromDir.cdUp();
        }
    }
    return dirs;
}

/**
   \brief Test a dir, try to create it if it does not exist.
   \param absPath : AbsolutePath of the directory to test
   \param createIfNotExist : try to create the dir if true
   \param logDirName : logical name to use for warning (eg : "Application path")
**/
bool checkDir(const QString & absPath, bool createIfNotExist, const QString & logDirName)
{
    if (! QFile::exists(absPath)) {
        if (createIfNotExist) {
            Utils::Log::addMessage("Utils", QCoreApplication::translate("Utils", "%1 : %2 does not exist. Trying to create it.")
                               .arg(logDirName, absPath));
            if (! QDir().mkpath(absPath)) {
                Utils::Log::addError("Utils", QCoreApplication::translate("Utils", "Unable to create the %1 : %2.")
                                 .arg(logDirName, absPath) ,
                                 __FILE__, __LINE__);
                return false;
            }
        } else {
            Utils::Log::addMessage("Utils", QCoreApplication::translate("Utils", "%1 : %2 does not exist.")
                               .arg(logDirName, absPath));
            return false;
        }
    }
    return true;
}

/** \brief Save the string to a text file. You can choose to warn the user or not is an error is encountered. Return true if all gone good. **/
bool saveStringToFile(const QString &toSave, const QString &toFile, IOMode iomode, const Warn warnUser, QWidget *parent)
{
    if (toFile.isEmpty()) {
        Utils::Log::addError("Utils", "saveStringToFile() : fileName is empty",
                              __FILE__, __LINE__);
        return false;
    }
    QWidget *wgt = parent;
    if (!parent) {
        wgt = qApp->activeWindow();
    }

    // Manage relative paths
    QString correctFileName = toFile;
    QFileInfo info(toFile);
    if (info.isRelative())
        correctFileName = qApp->applicationDirPath() + QDir::separator() + toFile;
    info.setFile(correctFileName);

    // Save file (overwrite)
    QFile file(info.absoluteFilePath());
    if (info.exists() && (info.isWritable() && warnUser == WarnUser)) {
        if (QMessageBox::warning(wgt, qApp->applicationName(),
                                   QCoreApplication::translate("Utils" ,
                                                                "File %1 already exists. Do you want de replace it ?").arg(info.fileName()),
                                   QMessageBox::Cancel | QMessageBox::Ok) == QMessageBox::Ok) {
            if (iomode == Overwrite) {
                if (!file.open(QFile::WriteOnly | QIODevice::Text)) {
                    Utils::Log::addError("Utils", QCoreApplication::translate("Utils", "Error %1 while trying to save file %2").arg(file.fileName(), file.errorString()),
                                         __FILE__, __LINE__);
                    return false;
                }
            } else if (iomode == AppendToFile) {
                if (!file.open(QFile::Append | QIODevice::Text)) {
                    Utils::Log::addError("Utils", QCoreApplication::translate("Utils", "Error %1 while trying to save file %2").arg(file.fileName(), file.errorString()),
                                         __FILE__, __LINE__);
                    return false;
                }
            } else {
                return false;
            }
            file.write(toSave.toAscii());
            Utils::Log::addMessage("Utils", QCoreApplication::translate("Utils", "%1 correctly saved").arg(file.fileName()));
        } else {
            Utils::Log::addMessage("Utils", QCoreApplication::translate("Utils", "Save file aborted by user (file already exists) : ") + file.fileName());
            return false;
        }
    } else {
        // Create file
        if (!file.open(QFile::WriteOnly | QIODevice::Text)) {
            Utils::Log::addError("Utils", QCoreApplication::translate("Utils", "Error %1 while trying to save file %2").arg(file.fileName(),file.errorString()),
                                  __FILE__, __LINE__);
            return false;
        }
        file.write(toSave.toAscii());
        Utils::Log::addMessage("Utils", QCoreApplication::translate("Utils", "%1 correctly saved").arg(file.fileName()));
    }
    return true;
}

/** \brief Save the string to a text file. Ask user for the name of the file to save. \sa  saveStringToFile() **/
bool saveStringToFile(const QString &toSave, const QString &dirPath, const QString &filters, QWidget *parent)
{
    QWidget *wgt = parent;
    if (!parent) {
        wgt = qApp->activeWindow();
    }
    QString fileName = QFileDialog::getSaveFileName(wgt, QCoreApplication::translate("Utils", "Save to file"),
                                                    dirPath,
                                                    filters);
    if (fileName.isEmpty())
        return false;
    return Utils::saveStringToFile(toSave, fileName, Overwrite, WarnUser, wgt);
}

QString readTextFile(const QString &toRead, const Warn warnUser, QWidget *parent)
{return readTextFile(toRead, QString("UTF-8"), warnUser, parent);}

/**
 \brief Return the content of a text file.
  You can choose:
  - to warn the user if an error is encountered.
  - the encoding of the file (using the QTextCodec supported encodings)
  - the parent widget used for the message box error
**/
QString readTextFile(const QString &toRead, const QString &encoder, const Warn warnUser, QWidget *parent)
{
    if (toRead.isEmpty())
        return QString();
    QWidget *p = parent;
    if (!p)
        p = qApp->activeWindow();

    // Manage relative paths
    QString correctFileName = toRead;
    QFileInfo info(toRead);
    if (info.isRelative())
        correctFileName = qApp->applicationDirPath() + QDir::separator() + toRead;
    info.setFile(correctFileName);

    if (((!info.exists()) || (!info.isReadable())) && (warnUser == WarnUser)) {
        Utils::warningMessageBox(QCoreApplication::translate("Utils" , "File %1 does not exists or is not readable.").arg(correctFileName),
                                 "","", qApp->applicationName());
        return QString::null;
    } else {
        QFile file(correctFileName);
        if (!file.open(QFile::ReadOnly | QIODevice::Text)) {
            Utils::Log::addError("Utils", QCoreApplication::translate("Utils", "Error %1 while trying to open file %2")
                             .arg(correctFileName, file.errorString()),
                             __FILE__, __LINE__);
            return QString::null;
        }
        QByteArray data = file.readAll();
        QTextCodec *codec = QTextCodec::codecForName(encoder.toUtf8());
        QString str = codec->toUnicode(data);
        Utils::Log::addMessage("Utils", tkTr(Trans::Constants::FILE_1_LOADED).arg(toRead));
        return str;
    }
    return QString::null;
}

/** \brief Test a dir, if dir exists return the absDirName, otherwise return empty QString. **/
QString isDirExists(const QString &absPath)
{
    if (QDir(absPath).exists())
        return QDir::cleanPath(absPath);
    return QString();
}

/** \brief Test a file, if file exists return the absFileName, otherwise return empty QString. **/
QString isFileExists(const QString &absPath)
{
    if (QFile(absPath).exists())
        return QDir::cleanPath(absPath);
    return QString();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////   MESSAGEBOXES FUNCTIONS   //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief Creates an informative messagebox. **/
void informativeMessageBox(const QString &text, const QString &infoText, const QString &detail, const QString &title)
{
    QWidget *parent = qApp->activeWindow();
    QMessageBox mb(parent);
    mb.setWindowModality(Qt::WindowModal);
    mb.setIcon(QMessageBox::Information);
    if (title.isEmpty())
        mb.setWindowTitle(qApp->applicationName());
    else
        mb.setWindowTitle(title);
    mb.setText(text);
    mb.setInformativeText(infoText);
    if (!detail.isEmpty()) {
        if (Qt::mightBeRichText(detail)) {
            QTextDocument doc;
            doc.setHtml(detail);
            mb.setDetailedText(doc.toPlainText());
        } else {
            mb.setDetailedText(detail);
        }
    }
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Ok);
    mb.exec();
    qApp->setActiveWindow(parent);
}

/** \brief Creates a warning messagebox. **/
void warningMessageBox(const QString &text, const QString &infoText, const QString &detail, const QString &title)
{
    Utils::Log::addMessage("Warning Dialog", infoText);
    QWidget *parent = qApp->activeWindow();
    QMessageBox mb(parent);
    mb.setWindowModality(Qt::WindowModal);
    mb.setIcon(QMessageBox::Warning);
    if (title.isEmpty())
        mb.setWindowTitle(qApp->applicationName());
    else
        mb.setWindowTitle(title);
    mb.setText(text);
    mb.setInformativeText(infoText);
    if (!detail.isEmpty()) {
        if (Qt::mightBeRichText(detail)) {
            QTextDocument doc;
            doc.setHtml(detail);
            mb.setDetailedText(doc.toPlainText());
        } else {
            mb.setDetailedText(detail);
        }
    }
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Ok);
    mb.exec();
    qApp->setActiveWindow(parent);
}

/** \brief Creates a messagebox with yes / no. Return true if user clicked yes. **/
bool yesNoMessageBox(const QString &text, const QString&infoText, const QString&detail, const QString &title, const QPixmap &icon)
{
    QWidget *parent = qApp->activeWindow();
    QMessageBox mb(parent);
    mb.setWindowModality(Qt::WindowModal);
    if (icon.isNull()) {
        mb.setIcon(QMessageBox::Question);
    } else {
        mb.setIconPixmap(icon);
    }
    if (title.isEmpty())
        mb.setWindowTitle(qApp->applicationName());
    else
        mb.setWindowTitle(title);
    mb.setText(text);
    mb.setInformativeText(infoText);
    if (!detail.isEmpty()) {
        if (Qt::mightBeRichText(detail)) {
            QTextDocument doc;
            doc.setHtml(detail);
            mb.setDetailedText(doc.toPlainText());
        } else {
            mb.setDetailedText(detail);
        }
    }
    mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    mb.setDefaultButton(QMessageBox::Yes);
    int r = mb.exec();
    qApp->setActiveWindow(parent);
    if (r==QMessageBox::Yes)
        return true;
    return false;
}

/**
  \brief Creates a messagebox with many buttons.
  Return -1 if dialog was cancelled, or the index of the button into the stringlist.
**/
int withButtonsMessageBox(const QString &text, const QString&infoText, const QString&detail, const QStringList &buttonsText, const QString &title, bool withCancelButton)
{
    QWidget *parent = qApp->activeWindow();
    QMessageBox mb(parent);
    mb.setWindowModality(Qt::WindowModal);
    mb.setIcon(QMessageBox::Question);
    if (title.isEmpty())
        mb.setWindowTitle(qApp->applicationName());
    else
        mb.setWindowTitle(title);
    mb.setText(text);
    mb.setInformativeText(infoText);
    if (!detail.isEmpty()) {
        if (Qt::mightBeRichText(detail)) {
            QTextDocument doc;
            doc.setHtml(detail);
            mb.setDetailedText(doc.toPlainText());
        } else {
            mb.setDetailedText(detail);
        }
    }
    QList<QPushButton *> buttons;
    foreach(const QString &s, buttonsText) {
        buttons << mb.addButton(s, QMessageBox::YesRole);
    }
    if (withCancelButton) {
        buttons << mb.addButton(QApplication::translate("Utils", "Cancel"), QMessageBox::RejectRole);
    }
    mb.setDefaultButton(buttons.at(0));
    int r = mb.exec();
    qApp->setActiveWindow(parent);
    if (r==buttonsText.count())
        return -1;
    return buttons.indexOf(static_cast<QPushButton*>(mb.clickedButton()));
}

/**
  \brief Creates a messagebox with many standard buttons.
  Return the standard button selected.
**/
int withButtonsMessageBox(const QString &text, const QString&infoText, const QString&detail, QMessageBox::StandardButtons buts, QMessageBox::StandardButton defaultButton, const QString &title)
{
    QWidget *parent = qApp->activeWindow();
    QMessageBox mb(parent);
    mb.setWindowModality(Qt::WindowModal);
    mb.setIcon(QMessageBox::Question);
    if (title.isEmpty())
        mb.setWindowTitle(qApp->applicationName());
    else
        mb.setWindowTitle(title);
    mb.setText(text);
    mb.setInformativeText(infoText);
    if (!detail.isEmpty()) {
        if (Qt::mightBeRichText(detail)) {
            QTextDocument doc;
            doc.setHtml(detail);
            mb.setDetailedText(doc.toPlainText());
        } else {
            mb.setDetailedText(detail);
        }
    }
    mb.setStandardButtons(buts);
    mb.setDefaultButton(defaultButton);
    int r = mb.exec();
    qApp->setActiveWindow(parent);
    return r;
}

/** \brief Creates a messagebox with Ok / Cancel. Return true if user clicked ok. **/
bool okCancelMessageBox(const QString &text, const QString&infoText, const QString&detail, const QString &title)
{
    QWidget *parent = qApp->activeWindow();
    QMessageBox mb(parent);
    mb.setWindowModality(Qt::WindowModal);
    mb.setIcon(QMessageBox::Question);
    if (title.isEmpty())
        mb.setWindowTitle(qApp->applicationName());
    else
        mb.setWindowTitle(title);
    mb.setText(text);
    mb.setInformativeText(infoText);
    if (!detail.isEmpty()) {
        if (Qt::mightBeRichText(detail)) {
            QTextDocument doc;
            doc.setHtml(detail);
            mb.setDetailedText(doc.toPlainText());
        } else {
            mb.setDetailedText(detail);
        }
    }
    mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    mb.setDefaultButton(QMessageBox::Ok);
    int r = mb.exec();
    qApp->setActiveWindow(parent);
    return (r==QMessageBox::Ok);
}

/** \brief Creates a messagebox for non available function.  **/
bool functionNotAvailableMessageBox(const QString &functionText)
{
    informativeMessageBox(functionText,
                           QCoreApplication::translate("Utils","This function is not available in this version."),
                           QCoreApplication::translate("Utils","You can send an email to developpers and explain your difficulties : freemedforms@googlegroups.com.")
                        );
//                           .arg(qApp->organizationDomain()));
//                         .arg(tkSettings::instance()->path(tkSettings::WebSiteUrl)));
    return true;
}

/** \brief Shows a full screen quick debug dialog that shows each string of the list inside a textbrowser */
void quickDebugDialog(const QStringList &texts)
{
    QDialog *dlg = new QDialog();
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    QGridLayout *grid = new QGridLayout(dlg);

    foreach(const QString &s, texts) {
        QTextBrowser *f = new QTextBrowser(dlg);
        if (Qt::mightBeRichText(s))
            f->setHtml(s);
        else
            f->setPlainText(s);
        grid->addWidget(f);
    }
    grid->addWidget(buttonBox);
    dlg->connect(buttonBox, SIGNAL(accepted()), dlg, SLOT(accept()));
    setFullScreen(dlg,true);
    dlg->exec();
    delete buttonBox;
    delete dlg;
}

/** \brief Shows a default dialog with \e license terms and a small \e message. */
bool defaultLicenceAgreementDialog(const QString &message, Utils::LicenseTerms::AvailableLicense license)
{
    QDialog dlg;
    QGridLayout layout(&dlg);
    QDialogButtonBox buttonBox(QDialogButtonBox::Yes | QDialogButtonBox::No);
    QTextBrowser tbrowse(&dlg);
    tbrowse.setReadOnly(true);
    QLabel appname(&dlg);
    // Application name
    if (qApp->applicationName().isEmpty()) {
        dlg.setWindowTitle(QCoreApplication::translate("Utils", "License agreement acceptation"));
        appname.setText(QString("<b>%1</b>").arg(QCoreApplication::translate("Utils", "License agreement acceptation")));
    } else {
        dlg.setWindowTitle(qApp->applicationName());
        appname.setText(QString("<b>%1</b>").arg(qApp->applicationName()));
    }
    appname.setAlignment(Qt::AlignCenter);
    // Message
    QLabel centered;
    if (!message.isEmpty()) {
        centered.setText(message);
    } else {
        centered.setText(QCoreApplication::translate("Utils", "<b>Before you can use this software, you must agree its license terms</b>"));
    }
    QFont bold;
    bold.setBold(true);
    centered.setFont(bold);
    centered.setAlignment(Qt::AlignCenter);
    tbrowse.setText(Utils::LicenseTerms::getTranslatedLicenseTerms(license));
    // Question yes/no
    QLabel question(QCoreApplication::translate("Utils", "Do you agree these terms ?"));
    layout.addWidget(&appname);
    layout.addWidget(&centered);
    layout.addWidget(&tbrowse);
    layout.addWidget(&question);
    layout.addWidget(&buttonBox);
    dlg.connect(&buttonBox, SIGNAL(accepted()), &dlg, SLOT(accept()));
    dlg.connect(&buttonBox, SIGNAL(rejected()), &dlg, SLOT(reject()));
    dlg.show();
    qApp->setActiveWindow(&dlg);
    dlg.activateWindow();
    dlg.raise();
    if (dlg.exec()==QDialog::Accepted)
        return true;
    return false;
}

/** \brief Creates a Dialog for simple user's input.  **/
QString askUser(const QString &title, const QString &question)
{
    bool ok;
    QString text = QInputDialog::getText(qApp->activeWindow(), title, question, QLineEdit::Normal, "", &ok);
    if (ok)
        return text;
    return QString();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////   WIDGETS FUNCTIONS   /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief Center the widget into the desktop. **/
void centerWidget(QWidget *win, QWidget *reference)
{
    QPoint center;
    if (!reference) {
        reference = qApp->desktop();
        center = reference->rect().center();
    } else {
        center = reference->rect().center() + reference->pos();
    }
    QRect rect = win->rect();
    rect.moveCenter(center);
    win->move(rect.topLeft());
}

/** \brief Switch widget to fullscreen/non fullscreen. **/
void setFullScreen(QWidget* win, bool on)
{
    if (bool(win->windowState() & Qt::WindowFullScreen) == on)
        return;

    if (on) {
        win->setWindowState(win->windowState() | Qt::WindowFullScreen);
        Utils::Log::addMessage("mfGlobal", QCoreApplication::translate("Utils", "%1 is now in fullScreen Mode.").arg(win->objectName()));
        //statusBar()->hide();
        //menuBar()->hide();
    } else {
        win->setWindowState(win->windowState() & ~Qt::WindowFullScreen);
        Utils::Log::addMessage("mfGlobal", QCoreApplication::translate("Utils", "%1 is now in non fullScreen Mode.").arg(win->objectName()));
        //menuBar()->show();
        //statusBar()->show();
    }
}

void resizeAndCenter(QWidget *widget, QWidget *reference)
{
    // resize windows
    QWidget *ref = reference;
    if (!reference)
        ref = qApp->activeWindow();
    if (!ref) {
        widget->adjustSize();
        return;
    }
    QSize size = ref->size();
    size = QSize(size.width()*0.9, size.height()*0.9);
    widget->resize(size);
    // recenter window
    centerWidget(widget, ref);
}

/** \brief Switch echo mode af a lineEdit. **/
void switchEchoMode(QLineEdit * l)
{
    if (l->echoMode() == QLineEdit::Normal)
        l->setEchoMode(QLineEdit::Password);
    else
        l->setEchoMode(QLineEdit::Normal);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////   HTML FUNCTIONS   //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** \brief Return the CSS style for a font. **/
QString fontToHtml(const QFont &font, const QColor &color)
{
    QString style;
    style = QString("font-family:%1;").arg(font.family());
    style += QString("font-size:%1pt;").arg(font.pointSize());
    if (font.bold())
        style += "font-weight:bold;";
    else
        style += "font-weight:normal;";
    if (font.italic())
        style += "font-style:italic;";
    else
        style += "font-style:normal;";
    if (font.underline())
        style += "text-decoration:underline;";
    else
        style += "text-decoration:none;";
    if (color.isValid()) {
        style += QString("color:%1;").arg(color.name());
    }
    return style;
}

/** \brief Transform a Qt::Alignment to html equivalent. return things like : align="center" */
QString textAlignmentToHtml(const Qt::Alignment &align)
{
    QString toReturn;
    if (align & Qt::AlignCenter)
        toReturn = "center";
    else if (align & Qt::AlignJustify)
        toReturn = "justify";
    else if (align & Qt::AlignRight)
        toReturn = "right";
    else
        toReturn = "left";
    if (!toReturn.isEmpty()) {
        toReturn.prepend("align=\"");
        toReturn += "\" ";
    }
    return toReturn;
}

/** \brief Assumes a better encoding of HTML files by replacing special characters with the html code (é==&eacute;) **/
QString toHtmlAccent(const QString &html)
{
    if (html.isEmpty())
        return html;
    QString toReturn = html;
    QHash< QString, QString > accents;
    //    accents.insert(QString::fromUtf8("\""), "&quot;");
    accents.insert(QString::fromUtf8("é"), "&eacute;");
    accents.insert(QString(QChar(QChar::Nbsp)), "&nbsp;");

    accents.insert(QString::fromUtf8("è"), "&egrave;");
    accents.insert(QString::fromUtf8("à"), "&agrave;");
    accents.insert(QString::fromUtf8("ù"), "&ugrave;");

    accents.insert(QString::fromUtf8("ê"), "&ecirc;");
    accents.insert(QString::fromUtf8("â"), "&acirc;");
    accents.insert(QString::fromUtf8("î"), "&icirc;");
    accents.insert(QString::fromUtf8("ô"), "&ocirc;");
    accents.insert(QString::fromUtf8("û"), "&ucirc;");

    accents.insert(QString::fromUtf8("ë"), "&euml;");
    accents.insert(QString::fromUtf8("ï"), "&iuml;");
    accents.insert(QString::fromUtf8("ö"), "&ouml;");
    accents.insert(QString::fromUtf8("ü"), "&uuml;");

    accents.insert(QString::fromUtf8("œ"), "&oelig;");
    accents.insert(QString::fromUtf8("æ"), "&aelig;");
    accents.insert(QString::fromUtf8("ç"), "&ccedil;");

    accents.insert(QString::fromUtf8("ø"), "&oslash;");
    accents.insert(QString::fromUtf8("Ø"), "&Oslash;");

    accents.insert(QString::fromUtf8("É"), "&Eacute;");

    accents.insert(QString::fromUtf8("È"), "&Egrave;");
    accents.insert(QString::fromUtf8("À"), "&Agrave;");
    accents.insert(QString::fromUtf8("Ù"), "&Ugrave;");

    accents.insert(QString::fromUtf8("Ê"), "&Ecirc;");
    accents.insert(QString::fromUtf8("Â"), "&Acirc;");
    accents.insert(QString::fromUtf8("Î"), "&Icirc;");
    accents.insert(QString::fromUtf8("Ô"), "&Ocirc;");
    accents.insert(QString::fromUtf8("Û"), "&Ucirc;");

    accents.insert(QString::fromUtf8("Ë"), "&Euml;");
    accents.insert(QString::fromUtf8("Ï"), "&Iuml;");
    accents.insert(QString::fromUtf8("Ö"), "&Ouml;");
    accents.insert(QString::fromUtf8("Ü"), "&Uuml;");

    accents.insert(QString::fromUtf8("Œ"), "&OElig;");
    accents.insert(QString::fromUtf8("Æ"), "&AElig;");
    accents.insert(QString::fromUtf8("Ç"), "&Ccedil;");

    accents.insert(QString::fromUtf8("ø"), "&oslash;");
    accents.insert(QString::fromUtf8("Ø"), "&Oslash;");
    accents.insert(QString::fromUtf8("€"), "&#128;");
    accents.insert(QString::fromUtf8("¡"), "&iexcl;");

    accents.insert(QString::fromUtf8("¢"), "&cent;");
    accents.insert(QString::fromUtf8("£"), "&pound;");
    accents.insert(QString::fromUtf8("¤"), "&curren;");
    accents.insert(QString::fromUtf8("¥"), "&yen;");
    accents.insert(QString::fromUtf8("¦"), "&brvbar;");
    accents.insert(QString::fromUtf8("Ã"), "&Atilde;");
    accents.insert(QString::fromUtf8("µ"), "&micro;");
    accents.insert(QString::fromUtf8("·"), "&middot;");
    accents.insert(QString::fromUtf8("»"), "&raquo;");
    accents.insert(QString::fromUtf8("«"), "&laquo;");
    accents.insert(QString::fromUtf8("¼"), "&frac14;");
    accents.insert(QString::fromUtf8("½"), "&frac12;");
    accents.insert(QString::fromUtf8("¾"), "&frac34;");
    accents.insert(QString::fromUtf8("¿"), "&iquest;");
    accents.insert(QString::fromUtf8("÷"), "&divide;");
    accents.insert(QString::fromUtf8("•"), "&#149;");
    accents.insert(QString::fromUtf8("©"), "&copy;");
    accents.insert(QString::fromUtf8("®"), "&reg;");
    accents.insert(QString::fromUtf8("™"), "&#153;");
    accents.insert(QString::fromUtf8("§"), "&sect;");
    accents.insert(QString::fromUtf8("…"), "&#133;");
    accents.insert(QString::fromUtf8("ˆ"), "&#136;");
    accents.insert(QString::fromUtf8("—"), "&mdash;");

    foreach(const QString &k, accents.keys()) {
        toReturn.replace(k, accents.value(k));
    }
    return toReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////   XML FUNCTIONS   //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
  \brief Create a simple Xml content with a \e mainTag and a hash \e datas.
  You can specify the indentation of the resulting Xml.\n
  You can automatically encode to base64 the values of the tags.\n
  The \e mainTag represents the first englobing Xml tag of the output.\n
  The tags are added in an unordered way cause of the uses of the QHash.\n
  Usage :
  \code
      QHash<QString, QString> tag_value;
      tag_value.insert("Tag", "Value");
      tag_value.insert("Tag2", "Value2");
      QString xml = createXml("MyXmlFirstTag", tag_value, 4, true);
      // Result is :
      // <MyXmlFirstTag>
      //     <Tag>Base64OfValue</Tag>
      //     <Tag2>Base64OfValue2</Tag2>
      // </MyXmlFirstTag>
  \endcode
*/
QString createXml(const QString &mainTag, const QHash<QString,QString> &datas, const int indent,const bool valueToBase64 )
{
    QDomDocument doc;
    QDomElement main = doc.createElement(mainTag);
    doc.appendChild(main);
    if (valueToBase64) {
        foreach(const QString &k, datas.keys()) {
            QDomElement data  = doc.createElement(k);
            main.appendChild(data);
            if (!datas.value(k).isEmpty()) {
                QDomText dataText = doc.createTextNode(datas.value(k).toAscii().toBase64());
                data.appendChild(dataText);
            }
        }
    } else {
        foreach(const QString &k, datas.keys()) {
            QDomElement data  = doc.createElement(k);
            main.appendChild(data);
            if (!datas.value(k).isEmpty()) {
                QDomText dataText = doc.createTextNode(datas.value(k));
                data.appendChild(dataText);
            }
        }
    }
    return doc.toString(indent);
}

/**
  \brief Reads a Xml content. Content must be like the one produced by createXml(). The \e readDatas is cleared and filled.
  \sa createXml().
*/
bool readXml(const QString &xmlContent, const QString &generalTag, QHash<QString,QString> &readDatas, const bool valueFromBase64)
{
    if (!xmlContent.contains(generalTag)) {
        Utils::Log::addError("Utils::readXml",QString("Error while reading Xml : tag %1 not found").arg(generalTag),
                             __FILE__, __LINE__);
        return false;
    }
    readDatas.clear();

    QDomDocument doc;
    doc.setContent(xmlContent);
    QDomElement root = doc.documentElement();
    QDomElement paramElem = root.firstChildElement();

    if (valueFromBase64) {
        while (!paramElem.isNull()) {
            if (!paramElem.tagName().compare(generalTag, Qt::CaseInsensitive)) {
                paramElem = paramElem.nextSiblingElement();
                continue;
            }
            readDatas.insert(paramElem.tagName(), QByteArray::fromBase64(paramElem.text().trimmed().toAscii()));
            paramElem = paramElem.nextSiblingElement();
        }
    } else {
        while (!paramElem.isNull()) {
            if (!paramElem.tagName().compare(generalTag, Qt::CaseInsensitive)) {
                paramElem = paramElem.nextSiblingElement();
                continue;
            }
            readDatas.insert(paramElem.tagName(), paramElem.text().trimmed().toAscii());
            paramElem = paramElem.nextSiblingElement();
        }
    }

    return true;
}

QString xmlRead(const QDomElement &father, const QString &name, const QString &defaultValue)
{
    QDomElement elem = father.firstChildElement(name);

    if (elem.isNull())
        return defaultValue;

    return elem.text();
}

QString xmlRead(const QDomElement &father, const QString &name, const char *defaultValue)
{
    QString defaultStr(defaultValue);
    return Utils::xmlRead(father, name, defaultStr);
}

int xmlRead(const QDomElement &father, const QString &name, const int defaultValue)
{
    QString defaultStr = QString::number(defaultValue);
    QString strValue = Utils::xmlRead(father, name, defaultStr);
    bool ok;
    int val = strValue.toInt(&ok);
    if (ok)
        return val;
    else
        return defaultValue;
}

int xmlRead(const QDomElement &father, const QString &name, const long int defaultValue)
{
    QString defaultStr = QString::number(defaultValue);
    QString strValue = Utils::xmlRead(father, name, defaultStr);
    bool ok;
    long int val = strValue.toLong(&ok);
    if (ok)
        return val;
    else
        return defaultValue;
}

bool xmlRead(const QDomElement &father, const QString &name, const bool defaultValue)
{
    QString defaultStr = QString::number((int) defaultValue);
    QString strValue = Utils::xmlRead(father, name, defaultStr);
    bool ok;
    int val = strValue.toInt(&ok);
    if (ok)
        return val;
    else
        return defaultValue;
}

void xmlWrite(QDomElement &father, const QString &name, const QString &value)
{
    QDomDocument document = father.ownerDocument();

    QDomElement elem = document.createElement(name);
    father.appendChild(elem);

    QDomText t = document.createTextNode(value);
    elem.appendChild(t);
}

void xmlWrite(QDomElement &father, const QString &name, char *value)
{
    QString strValue(value);
    Utils::xmlWrite(father, name, strValue);
}

void xmlWrite(QDomElement &father, const QString &name, int value)
{
    QString valueStr = QString::number(value);
    Utils::xmlWrite(father, name, valueStr);
}

void xmlWrite(QDomElement &father, const QString &name, long int value)
{
    QString valueStr = QString::number(value);
    Utils::xmlWrite(father, name, valueStr);
}

void xmlWrite(QDomElement &father, const QString &name, bool value)
{
    QString valueStr = QString::number((int) value);
    Utils::xmlWrite(father, name, valueStr);
}

/** \brief Replace a token into a string. */
int replaceToken(QString &textToAnalyse, const QString &token, const QString &value)
{
    if (!textToAnalyse.contains(token))
        return 0;
    // replace all occurences of token : token must not contains [ and ]
    QString t = token;
    t.remove(Constants::TOKEN_OPEN);
    t.remove(Constants::TOKEN_CLOSE);
    int begin, end;
    begin = 0;
    end = 0;
    int beforeBegin, afterEnd;
    int tokenLength = token.length() + QString(Constants::TOKEN_OPEN).length() + QString(Constants::TOKEN_CLOSE).length();
    int toReturn = 0;
    while (true) {
        // Find '[TOKEN]'
        begin = textToAnalyse.indexOf(Constants::TOKEN_OPEN + token + Constants::TOKEN_CLOSE, begin);
        if (begin==-1)
            break;
        end = begin + tokenLength;
        // Find text before '[ BEFORE [TOKEN]]'
        beforeBegin = textToAnalyse.lastIndexOf(Constants::TOKEN_OPEN, begin - 1);
        // Find text after '[[TOKEN] AFTER ]'
        afterEnd = textToAnalyse.indexOf(Constants::TOKEN_CLOSE, end);
        if ((beforeBegin==-1) || (afterEnd==-1)) {
            Utils::Log::addError("Utils", QApplication::translate("Utils", "Token replacement error (%1). Wrong number of parentheses.")
                                                                .arg(token + QString::number(beforeBegin)),
                                                                __FILE__, __LINE__);
            begin = end;
            continue;
        }
        // Replace TOKEN and manage AFTER and BEFORE
        if (value.isEmpty()) {
            textToAnalyse.remove(beforeBegin, afterEnd-beforeBegin+1);
            ++toReturn;
        } else {
            QString before = textToAnalyse.mid(beforeBegin, begin-beforeBegin);
            QString after = textToAnalyse.mid(end, afterEnd-end);
//            qWarning() << beforeBegin << begin << before << end << afterEnd << after;
            textToAnalyse.remove(afterEnd,1);
            textToAnalyse.replace(begin, end-begin, value);
            textToAnalyse.remove(beforeBegin,1);
            ++toReturn;
        }
    }
    return toReturn;
}

int replaceTokens(QString &textToAnalyse, const QHash<QString, QString> &tokens_values)
{
    int i = 0;
    foreach(const QString &tok, tokens_values.keys()) {
        i += replaceToken(textToAnalyse, tok, tokens_values.value(tok));
    }
    return i;
}

/** First crypt string using SHA1 logarythm then transform crypted result to base64 (so it can be added into database without problem - no special characters). */
QString cryptPassword(const QString &toCrypt)
{
    QCryptographicHash crypter( QCryptographicHash::Sha1 );
    crypter.addData( toCrypt.toAscii() );
    return crypter.result().toBase64();
}

/** Crypt a clear login. */
QString loginForSQL(const QString &log)
{ return log.toAscii().toBase64(); }

/** Decrypt a crypted login. */
QString loginFromSQL(const QVariant &sql)
{ return QByteArray::fromBase64( sql.toByteArray() ); }

/** Decrypt a crypted login. */
QString loginFromSQL(const QString &sql)
{ return QByteArray::fromBase64(sql.toAscii()); }

} // End Utils
