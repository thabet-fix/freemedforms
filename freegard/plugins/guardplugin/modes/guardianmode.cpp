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
 *  Main Developers:   Eric MAEKER, <eric.maeker@gmail.com>                *
 *  Contributors:                                                          *
 *      NAME <MAIL@ADDRESS.COM>                                            *
 ***************************************************************************/
#include "guardianmode.h"
#include <guardplugin/constants.h>

#include <coreplugin/icore.h>
#include <coreplugin/iuser.h>
#include <coreplugin/itheme.h>
#include <coreplugin/isettings.h>
#include <coreplugin/imainwindow.h>
#include <coreplugin/constants_menus.h>
#include <coreplugin/constants_icons.h>
#include <coreplugin/modemanager/modemanager.h>

#include <utils/log.h>
#include <utils/global.h>
#include <translationutils/constants.h>
#include <translationutils/trans_account.h>

#include <QDebug>

using namespace Guard;
using namespace Internal;
using namespace Trans::ConstantTranslations;

static inline Core::ITheme *theme()  { return Core::ICore::instance()->theme(); }
static inline Core::ISettings *settings() { return Core::ICore::instance()->settings(); }
static inline Core::ModeManager *modeManager() {return Core::ICore::instance()->modeManager();}
static inline Core::IMainWindow *mainWindow() { return Core::ICore::instance()->mainWindow(); }

GuardianMode::GuardianMode(QObject *parent) :
    Core::IMode(parent)
{
    setDisplayName(tr("People"));
    setIcon(theme()->icon(Core::Constants::ICONUSERMANAGER, Core::ITheme::BigIcon));
    setPriority(Constants::P_MODE_GUARDIANS);
    setId(Constants::MODE_GUARDIANS);
    setPatientBarVisibility(false);

//    const QList<int> &context;
//    setContext();
//    m_Stack = new QStackedWidget;
//    setWidget(m_Stack);
    setWidget(new QWidget());
}

GuardianMode::~GuardianMode()
{
    qWarning() << "GuardianMode::~GuardianMode()"  ;
}

void GuardianMode::postCoreInitialization()
{
    if (Utils::Log::debugPluginsCreation())
        qWarning() << Q_FUNC_INFO;
}

#ifdef WITH_TESTS

void GuardianMode::test_runWidgetTests()
{
    //_widget->test_runAllTests();
}

#endif
