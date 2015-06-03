/* This file is part of the KDE project
   Copyright (C) 2003-2015 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KEXIUTILS_UTILS_H
#define KEXIUTILS_UTILS_H

#include "kexiutils_export.h"
#include "kexi_global.h"

#include <QPointer>
#include <QObject>
#include <QDateTime>
#include <QMetaMethod>
#include <QFont>
#include <QFrame>
#include <QFontDatabase>
#include <QMimeType>

#include <KIconLoader>

class QColor;
class QMetaProperty;
class QLayout;

//! @short General Utils
namespace KexiUtils
{

//! \return true if parent of \a o that is of type \a className or false otherwise
inline bool parentIs(QObject* o, const char* className = 0)
{
    if (!o)
        return false;
    while ((o = o->parent())) {
        if (o->inherits(className))
            return true;
    }
    return false;
}

//! \return parent object of \a o that is of type \a type or 0 if no such parent
template<class type>
inline type findParentByType(QObject* o)
{
    if (!o)
        return 0;
    while ((o = o->parent())) {
        if (dynamic_cast< type >(o))
            return dynamic_cast< type >(o);
    }
    return 0;
}

/*! \return first found child of \a o, inheriting \a className.
 If objName is 0 (the default), all object names match.
 Returned pointer type is casted. */
KEXIUTILS_EXPORT QObject* findFirstQObjectChild(QObject *o, const char* className, const char* objName);

/*! \return first found child of \a o, that inherit \a className.
 If \a objName is 0 (the default), all object names match.
 Returned pointer type is casted. */
template<class type>
inline type findFirstChild(QObject *o, const char* className, const char* objName = 0)
{
    return ::qobject_cast< type >(findFirstQObjectChild(o, className, objName));
}

//! Finds property for name \a name and object \a object returns it index;
//! otherwise returns a null QMetaProperty.
KEXIUTILS_EXPORT QMetaProperty findPropertyWithSuperclasses(const QObject* object,
        const char* name);

//! \return true is \a object object is of class name \a className
inline bool objectIsA(QObject* object, const char* className)
{
    return 0 == qstrcmp(object->metaObject()->className(), className);
}

//! \return true is \a object object is of the class names inside \a classNames
KEXIUTILS_EXPORT bool objectIsA(QObject* object, const QList<QByteArray>& classNames);

//! \return a list of methods for \a metaObject meta object.
//! The methods are of type declared in \a types and have access declared
//! in \a access.
KEXIUTILS_EXPORT QList<QMetaMethod> methodsForMetaObject(
    const QMetaObject *metaObject, QFlags<QMetaMethod::MethodType> types
    = QFlags<QMetaMethod::MethodType>(QMetaMethod::Method | QMetaMethod::Signal | QMetaMethod::Slot),
    QFlags<QMetaMethod::Access> access
    = QFlags<QMetaMethod::Access>(QMetaMethod::Private | QMetaMethod::Protected | QMetaMethod::Public));

//! Like \ref KexiUtils::methodsForMetaObject() but includes methods from all
//! parent meta objects of the \a metaObject.
KEXIUTILS_EXPORT QList<QMetaMethod> methodsForMetaObjectWithParents(
    const QMetaObject *metaObject, QFlags<QMetaMethod::MethodType> types
    = QFlags<QMetaMethod::MethodType>(QMetaMethod::Method | QMetaMethod::Signal | QMetaMethod::Slot),
    QFlags<QMetaMethod::Access> access
    = QFlags<QMetaMethod::Access>(QMetaMethod::Private | QMetaMethod::Protected | QMetaMethod::Public));

//! \return a list with all this class's properties.
KEXIUTILS_EXPORT QList<QMetaProperty> propertiesForMetaObject(
    const QMetaObject *metaObject);

//! \return a list with all this class's properties including thise inherited.
KEXIUTILS_EXPORT QList<QMetaProperty> propertiesForMetaObjectWithInherited(
    const QMetaObject *metaObject);

//! \return a list of enum keys for meta property \a metaProperty.
KEXIUTILS_EXPORT QStringList enumKeysForProperty(const QMetaProperty& metaProperty);

/*! Sets "wait" cursor with 1 second delay (or 0 seconds if noDelay is true).
 Does nothing if the application has no GUI enabled. (see KApplication::guiEnabled()) */
KEXIUTILS_EXPORT void setWaitCursor(bool noDelay = false);

/*! Remove "wait" cursor previously set with \a setWaitCursor(),
 even if it's not yet visible.
 Does nothing if the application has no GUI enabled. (see KApplication::guiEnabled()) */
KEXIUTILS_EXPORT void removeWaitCursor();

/*! Helper class. Allocate it in your code block as follows:
 <code>
 KexiUtils::WaitCursor wait;
 </code>
 .. and wait cursor will be visible (with one second delay) until you're in this block, without
 a need to call removeWaitCursor() before exiting the block.
 Does nothing if the application has no GUI enabled. (see KApplication::guiEnabled()) */
class KEXIUTILS_EXPORT WaitCursor
{
public:
    WaitCursor(bool noDelay = false);
    ~WaitCursor();
};

/*! Helper class. Allocate it in your code block as follows:
 <code>
 KexiUtils::WaitCursorRemover remover;
 </code>
 .. and the wait cursor will be hidden unless you leave this block, without
 a need to call setWaitCursor() before exiting the block. After leaving the codee block,
 the cursor will be visible again, if it was visible before creating the WaitCursorRemover object.
 Does nothing if the application has no GUI enabled. (see KApplication::guiEnabled()) */
class KEXIUTILS_EXPORT WaitCursorRemover
{
public:
    WaitCursorRemover();
    ~WaitCursorRemover();
private:
    bool m_reactivateCursor;
};

/*! \return filter string in QFileDialog format for a mime type pointed by \a mime
 If \a kdeFormat is true, QFileDialog-compatible filter string is generated,
 eg. "Image files (*.png *.xpm *.jpg)", otherwise KFileDialog -compatible
 filter string is generated, eg. "*.png *.xpm *.jpg|Image files (*.png *.xpm *.jpg)".
 "\\n" is appended if \a kdeFormat is true, otherwise ";;" is appended. */
KEXIUTILS_EXPORT QString fileDialogFilterString(const QMimeType &mime, bool kdeFormat = true);

/*! @overload QString fileDialogFilterString(const QMimeType &mime, bool kdeFormat = true) */
KEXIUTILS_EXPORT QString fileDialogFilterString(const QString& mimeName, bool kdeFormat = true);

/*! Like QString fileDialogFilterString(const QMimeType &mime, bool kdeFormat = true)
 but returns a list of filter strings. */
KEXIUTILS_EXPORT QString fileDialogFilterStrings(const QStringList& mimeStrings, bool kdeFormat);

/*! A global setting for minimal readable font.
 \a init is a widget that should be passed if no qApp->mainWidget() is available yet.
 The size of font is not smaller than the one returned by
 QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont). */
KEXIUTILS_EXPORT QFont smallFont(QWidget *init = 0);

/*! \return a color being a result of blending \a c1 with \a c2 with \a factor1
 and \a factor1 factors: (c1*factor1+c2*factor2)/(factor1+factor2). */
KEXIUTILS_EXPORT QColor blendedColors(const QColor& c1, const QColor& c2, int factor1 = 1, int factor2 = 1);

/*! \return a contrast color for a color \a c:
 If \a c is light color, darker color created using c.dark(200) is returned;
 otherwise lighter color created using c.light(200) is returned. */
KEXIUTILS_EXPORT QColor contrastColor(const QColor& c);

/*! \return a lighter color for a color \a c and a factor \a factor.
 For colors like Qt::red or Qt::green where hue and saturation are near to 255,
 hue is decreased so the result will be more bleached.
 For black color the result is dark gray rather than black. */
KEXIUTILS_EXPORT QColor bleachedColor(const QColor& c, int factor);

/*! \return icon set computed as a result of colorizing \a icon pixmap with \a role
 color of \a palette palette. This function is useful for displaying monochromed icons
 on the list view or table view header, to avoid bloat, but still have the color compatible
 with accessibility settings. */
KEXIUTILS_EXPORT QIcon colorizeIconToTextColor(const QPixmap& icon, const QPalette& palette,
                                               QPalette::ColorRole role = QPalette::ButtonText);

/*! Replaces colors in pixmap @a original using @a color color. Used for coloring bitmaps 
 that have to reflect the foreground color. */
KEXIUTILS_EXPORT void replaceColors(QPixmap* original, const QColor& color);

/*! Replaces colors in image @a original using @a color color. Used for coloring bitmaps 
 that have to reflect the foreground color. */
KEXIUTILS_EXPORT void replaceColors(QImage* original, const QColor& color);

/*! @return true if curent color scheme is light.
 Lightness of window background is checked to measure this. */
KEXIUTILS_EXPORT bool isLightColorScheme();

/*! @return alpha value for dimmed color (150). */
KEXIUTILS_EXPORT int dimmedAlpha();

/*! @return palette @a pal with dimmed color @a role. @see dimmedAlpha() */
KEXIUTILS_EXPORT QPalette paletteWithDimmedColor(const QPalette &pal,
                                                 QPalette::ColorGroup group,
                                                 QPalette::ColorRole role);

/*! @overload paletteWithDimmedColor(const QPalette &, QPalette::ColorGroup, QPalette::ColorRole) */
KEXIUTILS_EXPORT QPalette paletteWithDimmedColor(const QPalette &pal,
                                                 QPalette::ColorRole role);

/*! @return palette altered for indicating "read only" flag. */
KEXIUTILS_EXPORT QPalette paletteForReadOnly(const QPalette &palette);

/*! \return empty (fully transparent) pixmap that can be used as a place for icon of size \a iconGroup */
KEXIUTILS_EXPORT QPixmap emptyIcon(KIconLoader::Group iconGroup);

#ifdef KEXI_DEBUG_GUI
//! Creates debug window for convenient debugging output
KEXIUTILS_EXPORT QWidget *createDebugWindow(QWidget *parent);

//! Connects push button action to \a receiver and its \a slot. This allows to execute debug-related actions
//! using buttons displayed in the debug window.
KEXIUTILS_EXPORT void connectPushButtonActionForDebugWindow(const char* actionName,
        const QObject *receiver, const char* slot);
#endif

//! Sets focus for widget \a widget with reason \a reason.
KEXIUTILS_EXPORT void setFocusWithReason(QWidget* widget, Qt::FocusReason reason);

//! Unsets focus for widget \a widget with reason \a reason.
KEXIUTILS_EXPORT void unsetFocusWithReason(QWidget* widget, Qt::FocusReason reason);

//! @short A convenience class that simplifies usage of QWidget::getContentsMargins() and QWidget::setContentsMargins
class KEXIUTILS_EXPORT WidgetMargins
{
public:
    //! Creates object with all margins set to 0
    WidgetMargins();
    //! Creates object with margins copied from \a widget
    explicit WidgetMargins(QWidget *widget);
    //! Creates object with margins set to given values
    WidgetMargins(int _left, int _top, int _right, int _bottom);
    //! Creates object with all margins set to commonMargin
    explicit WidgetMargins(int commonMargin);
    //! Copies margins from \a widget to this object
    void copyFromWidget(QWidget *widget);
    //! Creates margins from this object copied to \a widget
    void copyToWidget(QWidget *widget);
    //! Adds the given margins \a margins to this object, and returns a reference to this object
    WidgetMargins& operator+= (const WidgetMargins& margins);

    int left, top, right, bottom;
};

//! \return the sum of \a margins1 and \a margins1; each component is added separately.
const WidgetMargins operator+ (const WidgetMargins& margins1, const WidgetMargins& margins2);

//! Draws pixmap @a pixmap on painter @a p using predefined parameters.
//! Used in KexiDBImageBox and KexiBlobTableEdit.
KEXIUTILS_EXPORT void drawPixmap(QPainter& p, const WidgetMargins& margins, const QRect& rect,
                                 const QPixmap& pixmap, Qt::Alignment alignment, bool scaledContents, bool keepAspectRatio,
                                 Qt::TransformationMode transformMode = Qt::FastTransformation);

//! Scales pixmap @a pixmap on painter @a p using predefined parameters.
//! Used in KexiDBImageBox and KexiBlobTableEdit.
KEXIUTILS_EXPORT QPixmap scaledPixmap(const WidgetMargins& margins, const QRect& rect,
                                      const QPixmap& pixmap, QPoint& pos, Qt::Alignment alignment,
                                      bool scaledContents, bool keepAspectRatio,
                                      Qt::TransformationMode transformMode = Qt::FastTransformation);

//! A helper for automatic deleting of contents of containers.
template <typename Container>
class ContainerDeleter
{
public:
    explicit ContainerDeleter(Container& container) : m_container(container) {}
    ~ContainerDeleter() {
        clear();
    }
    void clear() {
        qDeleteAll(m_container); m_container.clear();
    }
private:
    Container& m_container;
};

//! Helper that sets given variable to specified value on destruction
//! Object of type Setter are supposed to be created on the stack.
template <typename T>
class Setter
{
public:
    //! Creates a new setter object for variable @a var,
    //! which will be set to value @a val on setter's destruction.
    Setter(T* var, const T& val)
        : m_var(var), m_value(val)
    {
    }
    ~Setter() {
        if (m_var)
            *m_var = m_value;
    }
    //! Clears the assignment, so the setter
    //! will not alter the variable on destruction
    void clear() { m_var = 0; }
private:
    T* m_var;
    const T m_value;
};

/*! A modified QFrame which sets up sunken styled panel frame style depending
 on the current widget style. The widget also reacts on style changes. */
class KEXIUTILS_EXPORT KTextEditorFrame : public QFrame
{
public:
    explicit KTextEditorFrame(QWidget * parent = 0, Qt::WindowFlags f = 0);
protected:
    virtual void changeEvent(QEvent *event);
};

/**
 * Returns the number of pixels that should be used between a
 * dialog edge and the outermost widget(s) according to the KDE standard.
 *
 * Copied from QDialog.
 *
 * @deprecated Use the style's pixelMetric() function to query individual margins.
 * Different platforms may use different values for the four margins.
 */
KEXIUTILS_EXPORT int marginHint();

/**
 * Returns the number of pixels that should be used between
 * widgets inside a dialog according to the KDE standard.
 *
 * Copied from QDialog.
 *
 * @deprecated Use the style's layoutSpacing() function to query individual spacings.
 * Different platforms may use different values depending on widget types and pairs.
 */
KEXIUTILS_EXPORT int spacingHint();

/*! Sets KexiUtils::marginHint() margins and KexiUtils::spacingHint() spacing
 for the layout @a layout. */
KEXIUTILS_EXPORT void setStandardMarginsAndSpacing(QLayout *layout);

/*! Sets the same @a value for layout @a layout margins. */
KEXIUTILS_EXPORT void setMargins(QLayout *layout, int value);

//! sometimes we leave a space in the form of empty QFrame and want to insert here
//! a widget that must be instantiated by hand.
//! This macro inserts a widget \a what into a frame \a where.
#define GLUE_WIDGET(what, where) \
    { QVBoxLayout *lyr = new QVBoxLayout(where); \
        lyr->addWidget(what); }

//! A tool for setting temporary value for boolean variable.
/*! After desctruction of the instance, the variable is set back
 to the original value. This class is useful in recursion guards.
 To use it, declare class atrribute of type bool and block it, e.g.:
 @code
 bool m_myNonRecursiveFunctionEnabled;
 // ... set m_myNonRecursiveFunctionEnabled initially to true
 void myNonRecursiveFunctionEnabled() {
    if (!m_myNonRecursiveFunctionEnabled)
        return;
    kexiUtils::BoolBlocker guard(m_myNonRecursiveFunctionEnabled, false);
    // function's body guarded against recursion...
 }
 @endcode
*/
class KEXIUTILS_EXPORT BoolBlocker
{
public:
    inline BoolBlocker(bool& var, bool tempValue)
     : v(var), origValue(var) { var = tempValue; }
    inline ~BoolBlocker() { v = origValue; }
private:
    bool& v;
    bool origValue;
};

/*! This helper function install an event filter on @a object and all of its
  children, directed to @a filter. */
KEXIUTILS_EXPORT void installRecursiveEventFilter(QObject *object, QObject *filter);

/*! This helper function removes an event filter installed before
  on @a object and all of its children. */
KEXIUTILS_EXPORT void removeRecursiveEventFilter(QObject *object, QObject *filter);

//! Blocks paint events on specified widget.
/*! Works recursively. Useful when widget should be hidden without changing
    geometry it takes. */
class KEXIUTILS_EXPORT PaintBlocker : public QObject
{
public:
    explicit PaintBlocker(QWidget* parent);
    void setEnabled(bool set);
    bool enabled() const;
protected:
    virtual bool eventFilter(QObject* watched, QEvent* event);
private:
    bool m_enabled;
};

/*!
 * \short Options for opening of hyperlinks
 * \sa openHyperLink()
 */
class KEXIUTILS_EXPORT OpenHyperlinkOptions : public QObject
{
    Q_OBJECT
    Q_ENUMS(HyperlinkTool)

public:

    /*!
     * A tool used for opening hyperlinks
     */
    enum HyperlinkTool{
        DefaultHyperlinkTool, /*!< Default tool for a given type of the hyperlink */
        BrowserHyperlinkTool, /*!< Opens hyperlink in a browser */
        MailerHyperlinkTool /*!< Opens hyperlink in a default mailer */
    };

    OpenHyperlinkOptions() :
        tool(DefaultHyperlinkTool)
      , allowExecutable(false)
      , allowRemote(false)
    {}

    HyperlinkTool tool;
    bool allowExecutable;
    bool allowRemote;
};

/*!
 * Opens the given \a url using \a options
*/
KEXIUTILS_EXPORT void openHyperLink(const QUrl &url, QWidget *parent,
                                    const OpenHyperlinkOptions &options);

//! \return size of combo box arrow according to \a style
/*! Application's style is the default. \see QStyle::SC_ComboBoxArrow */
KEXIUTILS_EXPORT QSize comboBoxArrowSize(QStyle *style = 0);

//! Adds a dirty ("document modified") flag to @a text according to current locale.
//! It is usually "*" character appended.
KEXIUTILS_EXPORT void addDirtyFlag(QString *text);

} //namespace KexiUtils

#endif //KEXIUTILS_UTILS_H
