/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2013 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
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
 *  Main Developers:                                                       *
 *       Eric MAEKER, <eric.maeker@gmail.com>,                             *
 *  Contributors:                                                          *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
#include "../alertplugin.h"
#include "../alertitem.h"
#include "../alertcore.h"
#include "../alertbase.h"

#include <coreplugin/icore.h>
#include <coreplugin/isettings.h>

#include <utils/log.h>
#include <utils/global.h>
#include <utils/randomizer.h>

//#include "alertitemeditordialog.h"
//#include "blockingalertdialog.h"
//#include "alertplaceholderwidget.h"

#include <QDir>
#include <QTest>

using namespace Alert;
using namespace Internal;

//static inline Core::ITheme *theme() {return Core::ICore::instance()->theme();}
//static inline Core::IUser *user() {return Core::ICore::instance()->user();}
static inline Core::ISettings *settings() {return Core::ICore::instance()->settings();}
static inline Alert::AlertCore *alertCore() {return Alert::AlertCore::instance();}
static inline Alert::Internal::AlertBase &alertBase() {return Alert::AlertCore::instance()->alertBase();}

namespace {
AlertScript &createVirtualScript(int scriptType, Utils::Randomizer &r)
{
    QString script = r.randomWords(r.randomInt(5, 25));
    script += "<=>+/-_;&\"'!:%*()|\\";
    AlertScript *alertscript = new AlertScript(Utils::createUid(), AlertScript::ScriptType(scriptType), script);
    alertscript->setId(r.randomInt(10000000));
    alertscript->setValid(r.randomBool());
    alertscript->setModified(false);
    return *alertscript;
}

// Creates a virtual item.
AlertItem &createVirtualItem(bool createAllRelations)
{
    Utils::Randomizer r;
    r.setPathToFiles(settings()->path(Core::ISettings::BundleResourcesPath) + "/textfiles/");

    QDir pix(settings()->path(Core::ISettings::SmallPixmapPath));

    AlertItem *item = new AlertItem;
    item->setValidity(r.randomBool());
    item->setUuid(Utils::createUid());
    if (r.randomBool())
        item->setCryptedPassword(r.randomWords(1).toUtf8().toBase64());

    // fr, de, en, xx
    QStringList langs;
    langs << "en" << "fr" << "de" << "xx";
    foreach(const QString &l, langs) {
        item->setLabel(r.randomWords(r.randomInt(2, 10)), l);
        item->setCategory(r.randomWords(r.randomInt(2, 10)), l);
        item->setDescription(r.randomWords(r.randomInt(2, 10)), l);
        item->setComment(r.randomWords(r.randomInt(2, 10)), l);
    }

    item->setViewType(AlertItem::ViewType(r.randomInt(0, AlertItem::NonBlockingAlert)));
    item->setContentType(AlertItem::ContentType(r.randomInt(0, AlertItem::UserNotification)));
    item->setPriority(AlertItem::Priority(r.randomInt(0, AlertItem::Low)));
    item->setCreationDate(r.randomDateTime(QDateTime::currentDateTime()));
    if (r.randomBool())
        item->setLastUpdate(r.randomDateTime(item->creationDate()));
    item->setThemedIcon(r.randomFile(pix, QStringList() << "*.png").fileName());
    if (r.randomBool())
        item->setStyleSheet(r.randomWords(10));
    if (r.randomBool())
        item->setExtraXml(QString("<xml>%1</xml>").arg(r.randomWords(r.randomInt(1, r.randomInt(2, 20))))); // one word at least otherwise QDocDocument can return </tag> instead of <tag></tag>

    // Add relations
    if (createAllRelations || r.randomBool()) {
        AlertRelation rel;
        rel.setRelatedTo(AlertRelation::RelatedToAllPatients);
        item->addRelation(rel);
    }
    if (createAllRelations || r.randomBool()) {
        AlertRelation rel;
        rel.setRelatedTo(AlertRelation::RelatedToPatient);
        rel.setRelatedToUid("patient1");
        item->addRelation(rel);
    }
    if (createAllRelations || r.randomBool()) {
        AlertRelation rel;
        rel.setRelatedTo(AlertRelation::RelatedToFamily);
        rel.setRelatedToUid("family1");
        item->addRelation(rel);
    }
    if (createAllRelations || r.randomBool()) {
        AlertRelation rel;
        rel.setRelatedTo(AlertRelation::RelatedToAllUsers);
        item->addRelation(rel);
    }
    if (createAllRelations || r.randomBool()) {
        AlertRelation rel;
        rel.setRelatedTo(AlertRelation::RelatedToUser);
        rel.setRelatedToUid("user1");
        item->addRelation(rel);
    }
    if (createAllRelations || r.randomBool()) {
        AlertRelation rel;
        rel.setRelatedTo(AlertRelation::RelatedToUserGroup);
        rel.setRelatedToUid("userGroup1");
        item->addRelation(rel);
    }
    if (createAllRelations || r.randomBool()) {
        AlertRelation rel;
        rel.setRelatedTo(AlertRelation::RelatedToApplication);
        rel.setRelatedToUid("application1");
        item->addRelation(rel);
    }

    // Add timing
    AlertTiming time;
    time.setValid(r.randomBool());
    time.setStart(r.randomDateTime(QDateTime::currentDateTime()));
    time.setEnd(time.start().addDays(r.randomInt(10, 5000)));
    if (r.randomBool()) {
        time.setCycling(true);
        time.setCyclingDelayInMinutes(r.randomInt(10*24*60, 1000*24*60));
        time.setNumberOfCycles(r.randomInt(1, 100));
    }
    item->addTiming(time);

    // Add scripts
    for(int i=0; i < AlertScript::OnRemindLater; ++i) {
        item->addScript(createVirtualScript(i, r));
    }

    // TODO : Add random validation
//    item->addValidation();

    item->setModified(false);
    return *item;
}
} // anonymous namespace

void AlertPlugin::test_alertitem_object()
{
    Utils::Randomizer r;
    r.setPathToFiles(settings()->path(Core::ISettings::BundleResourcesPath) + "/textfiles/");

    int loop = 1000;
    // Test AlertScript XML interface
    for(int i= 0; i < loop; ++i) {
        AlertScript script = createVirtualScript(2, r);
        QString xml = script.toXml();
        AlertScript script2 = AlertScript::fromXml(xml);
        QVERIFY(script.id() == script2.id());
        QVERIFY(script.uuid() == script2.uuid());
        QVERIFY(script.type() == script2.type());
        QVERIFY(script.script() == script2.script());
        QVERIFY(script.isNull() == script2.isNull());
        QVERIFY(script.isValid() == script2.isValid());
        QVERIFY(script2.isModified() == false);
    }

    // Test AlertValidation XML interface
    for(int i= 0; i < loop; ++i) {
        AlertValidation val;
        val.setAccepted(r.randomBool());
        val.setDateOfValidation(r.randomDateTime(QDateTime::currentDateTime()));
        val.setId(r.randomInt(1000));
        val.setOverriden(r.randomBool());
        val.setUserComment(r.randomWords(r.randomInt(10)));
        val.setValidatedUuid(Utils::createUid());
        val.setValidatorUuid(Utils::createUid());
        AlertValidation val2 = AlertValidation::fromXml(val.toXml());
        QVERIFY(val.id() == val2.id());
        QVERIFY(val.validatorUid() == val2.validatorUid());
        QVERIFY(val.userComment() == val2.userComment());
        QVERIFY(val.isOverriden() == val2.isOverriden());
        QVERIFY(val.isAccepted() == val2.isAccepted());
        QVERIFY(val.dateOfValidation() == val2.dateOfValidation());
        QVERIFY(val.validatedUid() == val2.validatedUid());
        QVERIFY(val2.isModified() == false);
    }

    // Test AlertTiming non-cycling XML interface
    for(int i= 0; i < loop; ++i) {
        AlertTiming t;
        t.setId(r.randomInt(1000));
        t.setValid(r.randomBool());
        QDateTime dt = QDateTime::currentDateTime();
        dt = r.randomDateTime(dt);
        t.setStart(r.randomDateTime(dt));
        dt.addDays(r.randomInt(1, r.randomInt(2, 20)));
        t.setEnd(dt);
        dt.addDays(r.randomInt(1, r.randomInt(2, 20)));
        t.setExpiration(dt);
        t.setCycling(false);
        AlertTiming t2 = AlertTiming::fromXml(t.toXml());
        QVERIFY(t.id() == t2.id());
        QVERIFY(t.isValid() == t2.isValid());
        QVERIFY(t.start() == t2.start());
        QVERIFY(t.end() == t2.end());
        QVERIFY(t.expiration() == t2.expiration());
        QVERIFY(t.isCycling() == t2.isCycling());
        QVERIFY(t.numberOfCycles() == t2.numberOfCycles());
        QVERIFY(t.nextDate() == t2.nextDate());
        QVERIFY(t.cyclingDelayInMinutes() == t2.cyclingDelayInMinutes());
        QVERIFY(t.cyclingDelayInHours() == t2.cyclingDelayInHours());
        QVERIFY(t.cyclingDelayInDays() == t2.cyclingDelayInDays());
        QVERIFY(t.cyclingDelayInWeeks() == t2.cyclingDelayInWeeks());
        QVERIFY(t.cyclingDelayInMonth() == t2.cyclingDelayInMonth());
        QVERIFY(t.cyclingDelayInYears() == t2.cyclingDelayInYears());
        QVERIFY(t.cyclingDelayInDecades() == t2.cyclingDelayInDecades());
        QVERIFY(t.cycleStartDate() == t2.cycleStartDate());
        QVERIFY(t.cycleExpirationDate() == t2.cycleExpirationDate());
        QVERIFY(t2.isModified() == false);
    }

    // Test AlertTiming cycling XML interface
    for(int i= 0; i < loop; ++i) {
        AlertTiming tc;
        tc.setId(r.randomInt(1000));
        tc.setValid(r.randomBool());
        QDateTime dt = QDateTime::currentDateTime();
        dt = r.randomDateTime(dt);
        tc.setStart(r.randomDateTime(dt));
        dt.addDays(r.randomInt(1, r.randomInt(2, 20)));
        tc.setEnd(dt);
        dt.addDays(r.randomInt(1, r.randomInt(2, 20)));
        tc.setExpiration(dt);
        tc.setCycling(true);
        tc.setNumberOfCycles(r.randomInt(1, 100)); // avoid 0 cycle...
        tc.setNextDate(r.randomDateTime(dt));
        tc.setCyclingDelayInMinutes(r.randomInt(100, 100000));
        tc.setCycleStartDate(r.randomDateTime(dt));
        dt.addDays(r.randomInt(10));
        tc.setCycleExpirationDate(dt);
        tc.setModified(false);
        AlertTiming tc2 = AlertTiming::fromXml(tc.toXml());
        QVERIFY(tc.id() == tc2.id());
        QVERIFY(tc.isValid() == tc2.isValid());
        QVERIFY(tc.start() == tc2.start());
        QVERIFY(tc.end() == tc2.end());
        QVERIFY(tc.expiration() == tc2.expiration());
        QVERIFY(tc.isCycling() == tc2.isCycling());
        QVERIFY(tc.numberOfCycles() == tc2.numberOfCycles());
        QVERIFY(tc.nextDate() == tc2.nextDate());
        QVERIFY(tc.cyclingDelayInMinutes() == tc2.cyclingDelayInMinutes());
        QVERIFY(tc.cyclingDelayInHours() == tc2.cyclingDelayInHours());
        QVERIFY(tc.cyclingDelayInDays() == tc2.cyclingDelayInDays());
        QVERIFY(tc.cyclingDelayInWeeks() == tc2.cyclingDelayInWeeks());
        QVERIFY(tc.cyclingDelayInMonth() == tc2.cyclingDelayInMonth());
        QVERIFY(tc.cyclingDelayInYears() == tc2.cyclingDelayInYears());
        QVERIFY(tc.cyclingDelayInDecades() == tc2.cyclingDelayInDecades());
        // Dynamic values
        // QVERIFY(tc.cycleStartDate() == tc2.cycleStartDate());
        // QVERIFY(tc.cycleExpirationDate() == tc2.cycleExpirationDate());
        QVERIFY(tc2.isModified() == false);
    }

    // Test AlertRelation XML interface
    for(int i= 0; i < loop; ++i) {
        AlertRelation rel;
        rel.setId(r.randomInt(1, 100000));
        rel.setRelatedTo(AlertRelation::RelatedTo(r.randomInt(0, AlertRelation::RelatedToApplication)));
        rel.setRelatedToUid(r.randomWords(1));
        AlertRelation rel2 = AlertRelation::fromXml(rel.toXml());
        QVERIFY(rel.id() == rel2.id());
        QVERIFY(rel.relatedTo() == rel2.relatedTo());
        QVERIFY(rel.relationTypeToString() == rel2.relationTypeToString());
        QVERIFY(rel.relatedToUid() == rel2.relatedToUid());
        QVERIFY(rel2.isModified() == false);
    }

    // Test the AlertItem interface
    for(int i=0; i < loop; ++i) {
        AlertItem item = createVirtualItem(true);
        AlertItem item2 = AlertItem::fromXml(item.toXml());
        QVERIFY(item.isValid() == item2.isValid());
        QVERIFY(item.uuid() == item2.uuid());
        QVERIFY(item.packUid() == item2.packUid());
        QVERIFY(item.cryptedPassword() == item2.cryptedPassword());
        QVERIFY(item.availableLanguages() == item2.availableLanguages());
        foreach(const QString &l, item.availableLanguages()) {
            QVERIFY(item.label(l) == item2.label(l));
            QVERIFY(item.toolTip(l) == item2.toolTip(l));
            QVERIFY(item.category(l) == item2.category(l));
            QVERIFY(item.description(l) == item2.description(l));
            QVERIFY(item.comment(l) == item2.comment(l));
        }
        QVERIFY(item.viewType() == item2.viewType());
        QVERIFY(item.contentType() == item2.contentType());
        QVERIFY(item.priority() == item2.priority());
        QVERIFY(item.isOverrideRequiresUserComment() == item2.isOverrideRequiresUserComment());
        QVERIFY(item.mustBeRead() == item2.mustBeRead());
        QVERIFY(item.isRemindLaterAllowed() == item2.isRemindLaterAllowed());
        QVERIFY(item.isEditable() == item2.isEditable());
        //QVERIFY(item.creationDate() == item2.creationDate());
        //QVERIFY(item.lastUpdate() == item2.lastUpdate());
        QVERIFY(item.themedIcon() == item2.themedIcon());
        QVERIFY(item.styleSheet() == item2.styleSheet());
        QVERIFY(item.priorityBackgroundColor() == item2.priorityBackgroundColor());
        QVERIFY(item.htmlToolTip(true) == item2.htmlToolTip(true));
        QVERIFY(item.htmlToolTip(false) == item2.htmlToolTip(false));

        if (item.extraXml() != item2.extraXml()) {
            qWarning() << item.extraXml() << item2.extraXml();
            qWarning() << item.toXml();
            qWarning() << item2.toXml();
        }

        QVERIFY(item.extraXml() == item2.extraXml());

        QVERIFY(item.relations().count() == item2.relations().count());
        QVERIFY(item.timings().count() == item2.timings().count());
        QVERIFY(item.scripts().count() == item2.scripts().count());
        QVERIFY(item.validations().count() == item2.validations().count());

        QVERIFY(item.remindLater() == item2.remindLater());
        QVERIFY(item.isUserValidated() == item2.isUserValidated());

        QVERIFY(item == item2);

        QVERIFY(item2.isModified() == false);
    }
}


void AlertPlugin::test_alertbase()
{
    // Test the AlertItem interface
    QDateTime start = QDateTime::currentDateTime().addSecs(-60*60*24);
    QDateTime expiration = QDateTime::currentDateTime().addSecs(60*60*24);

    AlertItem item; // = alertBase().createVirtualItem();
    item.setUuid(Utils::Database::createUid());
    item.setThemedIcon("identity.png");
    item.setLabel(item.label() + " (item)");
    item.setCategory("Test");
    item.setDescription("Simple basic non-blocking alert");
    item.setViewType(AlertItem::NonBlockingAlert);
    item.setPriority(AlertItem::High);
    item.setRemindLaterAllowed(true);
    item.clearRelations();
    item.addRelation(AlertRelation(AlertRelation::RelatedToPatient, "patient1"));
    item.clearTimings();
    item.addTiming(AlertTiming(start, expiration));

    AlertItem item2; // = alertBase().createVirtualItem();
    item2.setUuid(Utils::Database::createUid());
    item2.setThemedIcon("next.png");
    item2.setLabel("Just a simple alert (item2)");
    item2.setCategory("Test");
    item2.setDescription("Simple basic static alert");
    item2.setViewType(AlertItem::NonBlockingAlert);
    item2.setPriority(AlertItem::Low);
    item2.setOverrideRequiresUserComment(true);
    item2.clearRelations();
    item2.addRelation(AlertRelation(AlertRelation::RelatedToPatient, "patient1"));
    item2.addRelation(AlertRelation(AlertRelation::RelatedToPatient, "patient2"));
    item2.clearTimings();
    item2.addTiming(AlertTiming(start, expiration));

    AlertItem item3;
    item3.setUuid(Utils::Database::createUid());
    item3.setThemedIcon("ok.png");
    item3.setLabel("Just a simple alert (item3)");
    item3.setCategory("Test");
    item3.setDescription("Simple basic static alert that needs a user comment on overriding");
    item3.setViewType(AlertItem::NonBlockingAlert);
    item3.setPriority(AlertItem::Low);
    item3.setOverrideRequiresUserComment(true);
    item3.addRelation(AlertRelation(AlertRelation::RelatedToPatient, "patient1"));
    item3.addTiming(AlertTiming(start, expiration));

    AlertItem item4;
    item4.setUuid(Utils::Database::createUid());
    item4.setThemedIcon("elderly.png");
    item4.setLabel("Related to all patient (item4)");
    item4.setCategory("Test");
    item4.setDescription("Related to all patients and was validated for patient2 by user1.<br /> Static alert");
    item4.setViewType(AlertItem::NonBlockingAlert);
    item4.setPriority(AlertItem::Medium);
    item4.addRelation(AlertRelation(AlertRelation::RelatedToAllPatients));
    item4.addValidation(AlertValidation(QDateTime::currentDateTime(), "user1", "patient2"));
    item4.addTiming(AlertTiming(start, expiration));

    AlertItem item5;
    item5.setUuid(Utils::Database::createUid());
    item5.setLabel("Simple basic Blocking alert test (item5)");
    item5.setCategory("Test");
    item5.setDescription("Aoutch this is a Blocking alert !");
    item5.setViewType(AlertItem::BlockingAlert);
    item5.setRemindLaterAllowed(true);
    item5.setOverrideRequiresUserComment(true);
    item5.addRelation(AlertRelation(AlertRelation::RelatedToPatient, "patient1"));
    item5.addTiming(AlertTiming(start, expiration));

    AlertItem item6;
    item6.setUuid(Utils::Database::createUid());
    item6.setLabel("Simple basic Blocking user alert (item6)");
    item6.setCategory("Test user alert");
    item6.setDescription("Aoutch this is a Blocking alert !<br />For you, <b>user1</b>!");
    item6.setViewType(AlertItem::BlockingAlert);
    item6.setRemindLaterAllowed(true);
    item6.addRelation(AlertRelation(AlertRelation::RelatedToUser, "user1"));
    item6.addTiming(AlertTiming(start, expiration));
    item6.addScript(AlertScript("check_item6", AlertScript::CheckValidityOfAlert, "(1+1)==2;"));
    item6.addScript(AlertScript("onoverride_item6", AlertScript::OnOverridden, "(1+1)==2;"));

    AlertItem item7;
    item7.setUuid(Utils::Database::createUid());
    item7.setLabel("Simple basic alert (item7)");
    item7.setCategory("Test validated alert");
    item7.setDescription("Aoutch this is an error you should not see this !<br /><br />Validated for patient1.");
    item7.setViewType(AlertItem::NonBlockingAlert);
    item7.addRelation(AlertRelation(AlertRelation::RelatedToAllPatients));
    item7.addValidation(AlertValidation(QDateTime::currentDateTime(), "user1", "patient1"));
    item7.addTiming(AlertTiming(start, expiration));

    AlertItem item8;
    item8.setUuid(Utils::Database::createUid());
    item8.setLabel("Scripted alert (item8)");
    item8.setCategory("Test scripted alert");
    item8.setDescription("A valid alert with multiple scripts.");
    item8.setViewType(AlertItem::NonBlockingAlert);
    item8.addRelation(AlertRelation(AlertRelation::RelatedToAllPatients));
    item8.addTiming(AlertTiming(start, expiration));
    item8.addScript(AlertScript("check_item8", AlertScript::CheckValidityOfAlert, "(1+1)==2;"));

    AlertItem item9;
    item9.setUuid(Utils::Database::createUid());
    item9.setLabel("INVALID Scripted alert (item9)");
    item9.setCategory("Test scripted alert");
    item9.setDescription("A invalid alert with multiple scripts. YOU SHOULD NOT SEE IT !!!!");
    item9.setViewType(AlertItem::NonBlockingAlert);
    item9.addRelation(AlertRelation(AlertRelation::RelatedToAllPatients));
    item9.addTiming(AlertTiming(start, expiration));
    item9.addScript(AlertScript("check_item9", AlertScript::CheckValidityOfAlert, "(1+1)==3;"));

    AlertItem item10;
    item10.setUuid(Utils::Database::createUid());
    item10.setLabel("Cycling alert for all patients (item10)");
    item10.setCategory("Test cycling alert");
    item10.setDescription("Testing a cycling alert with:<br />- scripts<br />- relation to all patients.<br />Static alert.");
    item10.setViewType(AlertItem::NonBlockingAlert);
    item10.addRelation(AlertRelation(AlertRelation::RelatedToAllPatients));
    AlertTiming cycling(start, expiration);
    cycling.setCyclingDelayInYears(5);
    cycling.setNumberOfCycles(32565);
    item10.addTiming(cycling);
    item10.addScript(AlertScript("check_item10", AlertScript::CheckValidityOfAlert, "(1+1)==2;"));
    item10.addScript(AlertScript("startdate_item10", AlertScript::CyclingStartDate, "var currentDate = new Date(); currentDate.setDate(currentDate.getDate()-2); currentDate;"));

    AlertItem item11;
    item11.setUuid(Utils::Database::createUid());
    item11.setLabel("Test script interactions (item11)");
    item11.setCategory("Test script");
    item11.setDescription("Test script interaction with alertitem:<br />- redefine priority to HIGH<br />- change the label adding \"WAAAAAAHHHHHHOOUUUUHHH\"<br/>- add a comment");
    item11.setViewType(AlertItem::BlockingAlert);
    item11.addRelation(AlertRelation(AlertRelation::RelatedToAllPatients));
    item11.addTiming(AlertTiming(start, expiration));
    item11.addScript(AlertScript("check_item11", AlertScript::CheckValidityOfAlert,
                                 "print(\"CURRENT ALERT PROPERTY:\"+alert.priority());"
                                 "alert.setPriority(0);"
                                 "print(\"CURRENT ALERT LABEL:\"+alert.label());"
                                 "alert.setLabel(alert.label()+\"<br />WAAAAAAHHHHHHOOUUUUHHH\");"
                                 "alert.setComment(\"Niah niah niah comment added by the script...\");"
                                 "true;"));

    // Database saving
    QVERIFY(alertBase().saveAlertItem(item) == true);
    QVERIFY(alertBase().saveAlertItem(item2) == true);
    QVERIFY(alertBase().saveAlertItem(item3) == true);
    QVERIFY(alertBase().saveAlertItem(item4) == true);
    QVERIFY(alertBase().saveAlertItem(item5) == true);
    QVERIFY(alertBase().saveAlertItem(item6) == true);
    QVERIFY(alertBase().saveAlertItem(item7) == true);
    QVERIFY(alertBase().saveAlertItem(item8) == true);
    QVERIFY(alertBase().saveAlertItem(item9) == true);
    QVERIFY(alertBase().saveAlertItem(item10) == true);
    QVERIFY(alertBase().saveAlertItem(item11) == true);

    // Database getting
    Internal::AlertBaseQuery query;
    query.setAlertValidity(Internal::AlertBaseQuery::ValidAlerts);
    //        query.setAlertValidity(Internal::AlertBaseQuery::InvalidAlerts);
    query.addUserAlerts("user1");
    query.addUserAlerts("user2");
    query.addPatientAlerts("patient1");
    query.addPatientAlerts("patient2");
    query.addPatientAlerts("patient3");
    QVector<AlertItem> test = alertBase().getAlertItems(query);
    qWarning() << test.count();
    //        for(int i=0; i < test.count(); ++i) {
    //            qWarning() << "\n\n" << test.at(i).timingAt(0).start() << test.at(i).timingAt(0).end() << test.at(i).relationAt(1).relatedToUid();
    //        }
    //        qWarning() << "\n\n";
    //    AlertItem t = AlertItem::fromXml(item.toXml());
    //    qWarning() << (t.toXml() == item.toXml());

    // To XML
    if (false) {
        qWarning() << item.toXml();
        qWarning() << item11.toXml();
    }

    // Blocking alerts
//    if (false) {
//        item.setViewType(AlertItem::BlockingAlert);
//        item.setOverrideRequiresUserComment(true);
//        QToolButton *test = new QToolButton;
//        test->setText("Houlala");
//        test->setToolTip("kokokokokok");
//        QList<QAbstractButton*> buttons;
//        buttons << test;

//        BlockingAlertDialog::executeBlockingAlert(QList<AlertItem>() <<  item << item2 << item3 << item4 << item5, buttons);
//        //    BlockingAlertDialog::executeBlockingAlert(item4);
//    }

    // Alert editor
//    if (false) {
//        AlertItemEditorDialog dlg;
//        dlg.setEditableParams(AlertItemEditorDialog::FullDescription | AlertItemEditorDialog::Timing);
//        dlg.setEditableParams(AlertItemEditorDialog::Label | AlertItemEditorDialog::Timing);
//        dlg.setEditableParams(AlertItemEditorDialog::Label | AlertItemEditorDialog::Timing | AlertItemEditorDialog::Types);

//        AlertTiming &time = item.timingAt(0);
//        time.setCycling(true);
//        time.setCyclingDelayInDays(10);
//        dlg.setAlertItem(item);
//        if (dlg.exec()==QDialog::Accepted) {
//            dlg.submit(item);
//        }
//        qWarning() << item.toXml();
//    }

    // Alert packs
//    if (true) {
//        q->registerAlertPack(settings()->path(Core::ISettings::BundledAlertPacks) + "/test");
//    }

//    // PlaceHolders
//    if (true) {
//        // Put placeholder in the plugin manager object pool
//        _placeholdertest = new AlertPlaceHolderWidget; // object should not be deleted
//        pluginManager()->addObject(_placeholdertest);

//        // Create the dialog && the placeholder
//        QDialog dlg;
//        QVBoxLayout lay(&dlg);
//        dlg.setLayout(&lay);
//        lay.addWidget(_placeholdertest->createWidget(&dlg));

//        // Check alerts
//        q->checkAlerts(AlertCore::CurrentPatientAlerts | AlertCore::CurrentUserAlerts | AlertCore::CurrentApplicationAlerts);

//        // Exec the dialog
//        dlg.exec();
//    }

}

