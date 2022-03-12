/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2002-2003 Ariya Hidayat <ariya@kde.org>
             SPDX-FileCopyrightText: 2001-2003 Laurent Montel <montel@kde.org>
             SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

// Local
#include "ListDialog.h"

// #include <QGridLayout>
// #include <QLabel>
// #include <QListWidget>
// #include <QPushButton>

// #include <KSharedConfig>
// #include <kmessagebox.h>
// #include <ktextedit.h>

// #include "commands/AutoFillCommand.h"
// #include "Localization.h"

using namespace Calligra::Sheets;

class ListDialog::Private
{
public:
    KSharedConfigPtr config;

    QListWidget* list;
    KTextEdit* textEdit;
    QPushButton* addButton;
    QPushButton* cancelButton;
    QPushButton* removeButton;
    QPushButton* newButton;
    QPushButton* modifyButton;
    QPushButton* copyButton;
    bool changed;
};

static const int numBuiltinLists = 4;

ListDialog::ListDialog(QWidget* parent)
        : KoDialog(parent)
        , d(new Private)
{
    setCaption(i18n("Custom Lists"));
    setButtons(Ok | Cancel);
    setModal(true);

    QWidget* page = new QWidget(this);
    setMainWidget(page);

    QGridLayout *grid1 = new QGridLayout(page);

    QLabel *lab = new QLabel(page);
    lab->setText(i18n("List:"));
    grid1->addWidget(lab, 0, 0);

    d->list = new QListWidget(page);
    grid1->addWidget(d->list, 1, 0, 7, 1);

    lab = new QLabel(page);
    lab->setText(i18n("Entry:"));
    grid1->addWidget(lab, 0, 1);

    d->textEdit = new KTextEdit(page);
    grid1->addWidget(d->textEdit, 1, 1, 7, 1);

    d->addButton = new QPushButton(i18n("Add"), page);
    d->addButton->setEnabled(false);
    grid1->addWidget(d->addButton, 1, 2);

    d->cancelButton = new QPushButton(i18n("Cancel"), page);
    d->cancelButton->setEnabled(false);
    grid1->addWidget(d->cancelButton, 2, 2);

    d->newButton = new QPushButton(i18n("New"), page);
    grid1->addWidget(d->newButton, 3, 2);

    d->removeButton = new QPushButton(i18n("Remove"), page);
    grid1->addWidget(d->removeButton, 4, 2);

    d->modifyButton = new QPushButton(i18n("Modify"), page);
    grid1->addWidget(d->modifyButton, 5, 2);

    d->copyButton = new QPushButton(i18n("Copy"), page);
    grid1->addWidget(d->copyButton, 6, 2);

    connect(d->addButton, &QAbstractButton::clicked, this, &ListDialog::slotAdd);
    connect(d->cancelButton, &QAbstractButton::clicked, this, &ListDialog::slotCancel);
    connect(d->newButton, &QAbstractButton::clicked, this, &ListDialog::slotNew);
    connect(d->removeButton, &QAbstractButton::clicked, this, &ListDialog::slotRemove);
    connect(d->modifyButton, &QAbstractButton::clicked, this, &ListDialog::slotModify);
    connect(d->copyButton, &QAbstractButton::clicked, this, &ListDialog::slotCopy);
    connect(d->list, &QListWidget::itemDoubleClicked, this, &ListDialog::slotDoubleClicked);
    connect(d->list, &QListWidget::currentRowChanged, this, &ListDialog::slotCurrentRowChanged);
    connect(this, &KoDialog::okClicked, this, &ListDialog::slotOk);

    init();
    d->textEdit->setEnabled(false);
    d->modifyButton->setEnabled(false);
    slotCurrentRowChanged(0);
    resize(600, 250);
    d->changed = false;
}

ListDialog::~ListDialog()
{
    delete d;
}

void ListDialog::slotCurrentRowChanged(int row)
{
    //we can't remove the first built-in items
    const bool state = row >= numBuiltinLists;
    d->removeButton->setEnabled(state);
    d->copyButton->setEnabled(row >= 0);
    d->textEdit->setEnabled(false);
    d->textEdit->clear();
}

void ListDialog::init()
{
    QString month =
        i18n("January") + ", " +
        i18n("February") + ", " +
        i18n("March") + ", " +
        i18n("April") + ", " +
        i18n("May") + ", " +
        i18n("June") + ", " +
        i18n("July") + ", " +
        i18n("August") + ", " +
        i18n("September") + ", " +
        i18n("October") + ", " +
        i18n("November") + ", " +
        i18n("December");
    QStringList lst;
    lst.append(month);

    QString smonth =
        i18n("Jan") + ", " +
        i18n("Feb") + ", " +
        i18n("Mar") + ", " +
        i18n("Apr") + ", " +
        i18n("May") + ", " +
        i18n("Jun") + ", " +
        i18n("Jul") + ", " +
        i18n("Aug") + ", " +
        i18n("Sep") + ", " +
        i18n("Oct") + ", " +
        i18n("Nov") + ", ";
        i18n("Dec");
    lst.append(smonth);

    QString day =
        i18n("Monday") + ", " +
        i18n("Tuesday") + ", " +
        i18n("Wednesday") + ", " +
        i18n("Thursday") + ", " +
        i18n("Friday") + ", " +
        i18n("Saturday") + ", "+ 
        i18n("Sunday");
    lst.append(day);

    QString sday =
        i18n("Mon") + ", " +
        i18n("Tue") + ", " +
        i18n("Wed") + ", " +
        i18n("Thu") + ", " +
        i18n("Fri") + ", " +
        i18n("Sat") + ", " +
        i18n("Sun");
    lst.append(sday);

    d->config = KSharedConfig::openConfig();
    const QStringList other = d->config->group("Parameters").readEntry("Other list", QStringList());
    QString tmp;
    for (QStringList::ConstIterator it = other.begin(); it != other.end(); ++it) {
        if ((*it) != "\\") {
            tmp += (*it) + ", ";
        } else if (it != other.begin()) {
            tmp = tmp.left(tmp.length() - 2);
            lst.append(tmp);
            tmp.clear();
        }
    }
    d->list->addItems(lst);
}

void ListDialog::slotDoubleClicked()
{
    //we can't modify the first built-in items
    if (d->list->currentRow() < numBuiltinLists) {
        return;
    }
    const QStringList result = d->list->currentItem()->text().split(", ", QString::SkipEmptyParts);
    d->textEdit->setText(result.join(QChar('\n')));
    d->textEdit->setEnabled(true);
    d->modifyButton->setEnabled(true);
}

void ListDialog::slotAdd()
{
    d->addButton->setEnabled(false);
    d->cancelButton->setEnabled(false);
    d->newButton->setEnabled(true);
    d->list->setEnabled(true);
    const QStringList tmp = d->textEdit->toPlainText().split(QChar('\n'), QString::SkipEmptyParts);
    if (!tmp.isEmpty()) {
        d->list->addItem(tmp.join(", "));
    }

    d->textEdit->setText("");
    d->textEdit->setEnabled(false);
    d->textEdit->setFocus();
    slotCurrentRowChanged(0);
    d->changed = true;
}

void ListDialog::slotCancel()
{
    d->textEdit->setText("");
    slotAdd();
}

void ListDialog::slotNew()
{
    d->addButton->setEnabled(true);
    d->cancelButton->setEnabled(true);
    d->newButton->setEnabled(false);
    d->removeButton->setEnabled(false);
    d->modifyButton->setEnabled(false);
    d->copyButton->setEnabled(false);
    d->list->setEnabled(false);
    d->textEdit->setText("");
    d->textEdit->setEnabled(true);
    d->textEdit->setFocus();
}

void ListDialog::slotRemove()
{
    if (!d->list->isEnabled() || d->list->currentRow() == -1) {
        return;
    }
    //don't remove the first built-in items
    if (d->list->currentRow() < numBuiltinLists) {
        return;
    }
    int ret = KMessageBox::warningContinueCancel(this,
              i18n("Do you really want to remove this list?"),
              i18n("Remove List"), KStandardGuiItem::del());
    if (ret == Cancel) { // response = No
        return;
    }
    delete d->list->takeItem(d->list->currentRow());
    d->textEdit->setEnabled(false);
    d->textEdit->setText("");
    if (d->list->count() <= numBuiltinLists) {
        d->removeButton->setEnabled(false);
    }
    d->changed = true;
}

void ListDialog::slotOk()
{
    if (!d->textEdit->toPlainText().isEmpty()) {
        int ret = KMessageBox::warningYesNo(this, i18n("Entry area is not empty.\nDo you want to continue?"));
        if (ret == 4) { // response = No
            return;
        }
    }
    if (d->changed) {
        QStringList result;
        result.append("\\");

        //don't save the first built-in lines
        for (int i = numBuiltinLists - 1; i < d->list->count(); ++i) {
            QStringList tmp = d->list->item(i)->text().split(", ", QString::SkipEmptyParts);
            if (!tmp.isEmpty()) {
                result += tmp;
                result += "\\";
            }
        }
        d->config->group("Parameters").writeEntry("Other list", result);
        //todo refresh AutoFillCommand::other
        // I don't know how to do for the moment
        delete(AutoFillCommand::other);
        AutoFillCommand::other = 0;
    }
    accept();
}

void ListDialog::slotModify()
{
    //you can modify list but not the first built-in items
    if (d->list->currentRow() >= numBuiltinLists && !d->textEdit->toPlainText().isEmpty()) {
        const QString tmp = d->textEdit->toPlainText().split(QChar('\n'), QString::SkipEmptyParts).join(", ");
        d->list->insertItem(d->list->currentRow(), tmp);
        delete d->list->takeItem(d->list->currentRow());

        d->textEdit->setText("");
        d->changed = true;
    }
    d->textEdit->setEnabled(false);
    d->modifyButton->setEnabled(false);
}

void ListDialog::slotCopy()
{
    if (d->list->currentRow() != -1) {
        d->list->addItem(d->list->currentItem()->text());
    }
}
