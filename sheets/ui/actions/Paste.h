/* This file is part of the KDE project
   SPDX-FileCopyrightText: 1998-2023 The Calligra Team <calligra-devel@kde.org>
   SPDX-FileCopyrightText: 2023 Tomas Mecir <mecirt@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CALLIGRA_SHEETS_ACTION_PASTE
#define CALLIGRA_SHEETS_ACTION_PASTE


#include "CellAction.h"



namespace Calligra
{
namespace Sheets
{

class PasteRegular : public CellAction {
Q_OBJECT
public:
    PasteRegular(Actions *actions);
    virtual ~PasteRegular();

protected:
    virtual void execute(Selection *selection, Sheet *sheet, QWidget *canvasWidget) override;
    QAction *createAction() override;
};

class SpecialPasteDialog;

class PasteSpecial : public CellAction {
Q_OBJECT
public:
    PasteSpecial(Actions *actions);
    virtual ~PasteSpecial();

protected:
    virtual void execute(Selection *selection, Sheet *sheet, QWidget *canvasWidget) override;
    SpecialPasteDialog *m_dlg;
};

class PasteInsertDialog;

class PasteWithInsert : public CellAction {
Q_OBJECT
public:
    PasteWithInsert(Actions *actions);
    virtual ~PasteWithInsert();

protected:
    virtual void execute(Selection *selection, Sheet *sheet, QWidget *canvasWidget) override;
    PasteInsertDialog *m_dlg;
};



} // namespace Sheets
} // namespace Calligra

#endif   // CALLIGRA_SHEETS_ACTION_PASTE
