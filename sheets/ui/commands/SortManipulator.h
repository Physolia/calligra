// This file is part of the KDE project
// SPDX-FileCopyrightText: 2006 Tomas Mecir <mecirt@gmail.com>
// SPDX-License-Identifier: LGPL-2.0-only

#ifndef CALLIGRA_SHEETS_SORT_MANIPULATOR
#define CALLIGRA_SHEETS_SORT_MANIPULATOR

#include "DataManipulators.h"
#include "core/Cell.h"

namespace Calligra
{
namespace Sheets
{
class CellStorage;

/**
 * \ingroup Commands
 * \brief Sorts the values in a cell range.
 */
class CALLIGRA_SHEETS_UI_EXPORT SortManipulator : public AbstractDFManipulator
{
public:
    SortManipulator();
    ~SortManipulator() override;

    bool process(Element* element) override;

    /** true if rows are to be sorted, false if columns are */
    void setSortRows(bool v) {
        m_rows = v;
    }
    /** skip first row/column, as it contains headers */
    void setSkipFirst(bool v) {
        m_skipfirst = v;
    }
    /** set whether cell formats should be moved with data */
    void setCopyFormat(bool v) {
        setChangeFormat(v);
    }
    /** set whether we will use a custom list */
    void setUseCustomList(bool v) {
        m_usecustomlist = v;
    }
    /** set a custom list that will be used */
    void setCustomList(const QStringList &l) {
        m_customlist = l;
    }

    /** 
     * Adds a sort criterion.
     * Sort criteria are used in order in which they're added.
     * \param index the column/row index. Indexed from 0.
     * \param order sort order (ascending/descending)
     * \param caseSensitivity case sensitivity
     */
    void addCriterion(int index, Qt::SortOrder order, Qt::CaseSensitivity caseSensitivity);
    void clearCriteria();

protected:
    bool preProcessing() override;
    bool postProcessing() override;
    Value newValue(Element *element, int col, int row,
                           bool *parse, Format::Type *fmtType) override;
    Style newFormat(Element *element, int col, int row) override;

    /** sort the data, filling the "sorted" structure */
    void sort(Element *element);
    bool shouldReorder(Element *element, int first, int second);

    bool m_rows, m_skipfirst, m_usecustomlist;
    QStringList m_customlist;

    struct Criterion {
        int index;
        Qt::SortOrder order;
        Qt::CaseSensitivity caseSensitivity;
    };
    QList<Criterion> m_criteria;

    /** sorted order - which row/column will move to where */
    QMap<int, int> sorted;

    CellStorage* m_cellStorage; // temporary
    QHash<Cell, Style> m_styles; // temporary
    QHash<Cell, QString> m_formulas; // temporary; encoded formulas
};

} // namespace Sheets
} // namespace Calligra

#endif  // CALLIGRA_SHEETS_SORT_MANIPULATOR
