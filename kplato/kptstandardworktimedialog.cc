/* This file is part of the KDE project
   Copyright (C) 2004 - 2006 Dag Andersen <danders@get2net.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation;
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "kptstandardworktimedialog.h"
#include "kptduration.h"
#include "kptproject.h"
#include "kptcalendar.h"
#include "kptcommand.h"
#include "kptintervaledit.h"
#include "kptpart.h"

#include <QGroupBox>
#include <QHeaderView>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>

#include <kcalendarsystem.h>
#include <kdebug.h>
#include <klocale.h>
#include <knuminput.h>

namespace KPlato
{

class WeekdayListItem : public QTreeWidgetItem
{
public:
    WeekdayListItem(Calendar *cal, int wd, QTreeWidget *parent, const QString& name, QTreeWidgetItem *after)
    : QTreeWidgetItem(parent, after),
      original(cal->weekday(wd)),
      calendar(cal),
      weekday(wd)
    {
        setText(0, name);
        day = new CalendarDay(original);
        if (day->state() == Map::NonWorking) {
            setHours();
        } else {
            setText(1, KGlobal::locale()->formatNumber(day->duration().toDouble(Duration::Unit_h)));
        }
    }
    ~WeekdayListItem() {
        delete day;
    }
    void setHours() {
        setText(1, "-");
        day->clearIntervals();
    }
    void setIntervals(QList<TimeInterval*> intervals) {
        day->setIntervals(intervals);
        setText(1, KGlobal::locale()->formatNumber(day->duration().toDouble(Duration::Unit_h)));
    }
    void setState(int st) {
        day->setState(st+1);
    }
    
    KCommand *save(Part *part) {
        KCommand *cmd=0;
        if (*original != *day) {
            cmd = new CalendarModifyWeekdayCmd(part, calendar, weekday, day);
            day = 0;
        }
        return cmd;
    }
    CalendarDay *day;
    CalendarDay *original;
    Calendar *calendar;
    int weekday;
};

StandardWorktimeDialog::StandardWorktimeDialog(Project &p, QWidget *parent, const char *name)
    : KDialog(parent),
      project(p)
{
    setCaption( i18n("Standard Worktime") );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    showButtonSeparator( true );
    //kDebug()<<k_funcinfo<<&p<<endl;
    m_original = p.standardWorktime();
    dia = new StandardWorktimeDialogImpl(m_original, this);

    setMainWidget(dia);
    enableButtonOk(false);

    connect(dia, SIGNAL(obligatedFieldsFilled(bool) ), SLOT(enableButtonOk(bool)));
    connect(dia, SIGNAL(enableButtonOk(bool)), SLOT(enableButtonOk(bool)));
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
}

KMacroCommand *StandardWorktimeDialog::buildCommand(Part *part) {
    //kDebug()<<k_funcinfo<<endl;
    QString n = i18n("Modify Standard Worktime");
    KMacroCommand *cmd = 0;
    if (m_original->year() != dia->inYear()) {
        if (cmd == 0) cmd = new KMacroCommand(n);
        cmd->addCommand(new ModifyStandardWorktimeYearCmd(part, m_original, m_original->year(), dia->inYear()));
    }
    if (m_original->month() != dia->inMonth()) {
        if (cmd == 0) cmd = new KMacroCommand(n);
        cmd->addCommand(new ModifyStandardWorktimeMonthCmd(part, m_original, m_original->month(), dia->inMonth()));
    }
    if (m_original->week() != dia->inWeek()) {
        if (cmd == 0) cmd = new KMacroCommand(n);
        cmd->addCommand(new ModifyStandardWorktimeWeekCmd(part, m_original, m_original->week(), dia->inWeek()));
    }
    if (m_original->day() != dia->inDay()) {
        if (cmd == 0) cmd = new KMacroCommand(n);
        cmd->addCommand(new ModifyStandardWorktimeDayCmd(part, m_original, m_original->day(), dia->inDay()));
    }
    int cnt = dia->weekdayList->topLevelItemCount();
    for (int i=0; i < cnt; ++i) {
        KCommand *c = static_cast<WeekdayListItem*>(dia->weekdayList->topLevelItem(i))->save(part);
        if (c) {
            if (cmd == 0) cmd = new KMacroCommand(n);
            cmd->addCommand(c);
        }
    }
    return cmd;

}

void StandardWorktimeDialog::slotOk() {
    accept();
}


StandardWorktimeDialogImpl::StandardWorktimeDialogImpl(StandardWorktime *std, QWidget *parent)
    : QWidget(parent),
      m_std(std) {
    
    setupUi(this);
    if (!std) {
        m_std = new StandardWorktime();
    }
    QBoxLayout *l = new QVBoxLayout(intervalBox);
    m_intervalEdit = new IntervalEdit(intervalBox);
    l->addWidget(m_intervalEdit);
    
    weekdayList->setRootIsDecorated(false);
    
    m_year = m_std->year();
    m_month = m_std->month();
    m_week = m_std->week();
    m_day = m_std->day();

    kDebug()<<k_funcinfo<<"y="<<m_year<<" m="<<m_month<<" w="<<m_week<<" d="<<m_day<<endl;
    year->setRange(1.0, 8784.0, 0.1, 1);
    year->setValue(m_year);
    month->setRange(1.0, 744.0, 0.1, 1);
    month->setValue(m_month);
    week->setRange(1.0, 168.0, 0.1, 1);
    week->setValue(m_week);
    day->setRange(1.0, 24.0, 0.1, 1);
    day->setValue(m_day);
    weekdayList->setHeaderLabel(i18n("Weekday"));
    weekdayList->setSortingEnabled(false);
    weekdayList->header()->setStretchLastSection(true);
    const KCalendarSystem * cs = KGlobal::locale()->calendar();
    Calendar *cal = m_std->calendar();
    if (cal) {
        WeekdayListItem *item = 0;
        for (int i = 0; i < 7; ++i) {
            CalendarDay *day = cal->weekday(i);
            if (day == 0) {
                continue;
            }
            kDebug()<<k_funcinfo<<"Add day: "<<cs->weekDayName(i+1)<<endl;
            item = new WeekdayListItem(cal, i, weekdayList, cs->weekDayName(i+1), item);
            weekdayList->addTopLevelItem(item);
        }
    }

    connect(year, SIGNAL(valueChanged(double)), SLOT(slotYearChanged(double)));
    connect(month, SIGNAL(valueChanged(double)), SLOT(slotMonthChanged(double)));
    connect(week, SIGNAL(valueChanged(double)), SLOT(slotWeekChanged(double)));
    connect(day, SIGNAL(valueChanged(double)), SLOT(slotDayChanged(double)));

    connect(m_intervalEdit, SIGNAL(changed()), SLOT(slotIntervalChanged()));
    connect(bApply, SIGNAL(clicked()), SLOT(slotApplyClicked()));
    connect(weekdayList, SIGNAL(selectionChanged()), SLOT(slotWeekdaySelected()));
    connect(state, SIGNAL(activated(int)), SLOT(slotStateChanged(int)));
    
    if (weekdayList->topLevelItemCount() > 0) {
        weekdayList->topLevelItem(0)->setSelected(true);
        weekdayList->setCurrentItem(weekdayList->topLevelItem(0));
    }
}


void StandardWorktimeDialogImpl::slotEnableButtonApply(bool on) {
    bApply->setEnabled(on);
}

void StandardWorktimeDialogImpl::slotEnableButtonOk(bool on) {
    emit enableButtonOk(on);
}

void StandardWorktimeDialogImpl::slotCheckAllFieldsFilled() {
    emit obligatedFieldsFilled(true);
}

void StandardWorktimeDialogImpl::slotYearChanged(double value) {
    //kDebug()<<k_funcinfo<<value<<endl;
    m_year = value;
    if (month->value() > value)
        month->setValue(value);
    slotEnableButtonOk(true);
}

void StandardWorktimeDialogImpl::slotMonthChanged(double value) {
    m_month = value;
    if (year->value() < value)
        year->setValue(value);
    if (week->value() > value)
        week->setValue(value);
    slotEnableButtonOk(true);
}

void StandardWorktimeDialogImpl::slotWeekChanged(double value) {
    m_week = value;
    if (month->value() < value)
        month->setValue(value);
    if (day->value() > value)
        day->setValue(value);
    slotEnableButtonOk(true);
}

void StandardWorktimeDialogImpl::slotDayChanged(double value) {
    m_day = value;
    if (week->value() < value)
        week->setValue(value);
    slotEnableButtonOk(true);
}

void StandardWorktimeDialogImpl::slotIntervalChanged() {
    //kDebug()<<k_funcinfo<<endl;
    slotEnableButtonApply(true);
}

void StandardWorktimeDialogImpl::slotApplyClicked() {
    //kDebug()<<k_funcinfo<<"state="<<state->currentItem()<<endl;
    int c = weekdayList->topLevelItemCount();
    for (int i=0; i < c; ++i) {
        QTreeWidgetItem *item = weekdayList->topLevelItem(i);
        if (item->isSelected()) {
            //kDebug()<<k_funcinfo<<item->text(0)<<" selected"<<endl;
            WeekdayListItem *wd = static_cast<WeekdayListItem*>(item);
            wd->setState(state->currentItem());
            if (state->currentItem() == 0) {
                wd->setHours();
            } else {
                wd->setIntervals(m_intervalEdit->intervals());
            }
            slotEnableButtonOk(true);
        }
    }
}

void StandardWorktimeDialogImpl::slotWeekdaySelected() {
    //kDebug()<<k_funcinfo<<"state="<<state->currentItem()<<endl;
    
    QList<QTreeWidgetItem *> lst = weekdayList->selectedItems();
    if (lst.count() == 0) {
        editBox->setEnabled(false);
        return;
    }
    //kDebug()<<k_funcinfo<<item->text(0)<<" selected"<<endl;
    WeekdayListItem *wd = static_cast<WeekdayListItem*>(lst[0]);
    state->setCurrentIndex(wd->day->state()-1);
    m_intervalEdit->setIntervals(wd->day->workingIntervals());
    slotStateChanged(state->currentItem());
    editBox->setEnabled(true);
}

void StandardWorktimeDialogImpl::slotStateChanged(int st) {
    //kDebug()<<k_funcinfo<<"state="<<state->currentItem()<<endl;
    intervalBox->setEnabled(st == 1); //Working
    slotEnableButtonApply(st == 0);
}

}  //KPlato namespace


#include "kptstandardworktimedialog.moc"
