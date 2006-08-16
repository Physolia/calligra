#ifndef TESTDOCUMENTAYOUT_H
#define TESTDOCUMENTAYOUT_H

#include <QObject>
#include <QtTest/QtTest>

#include <KoTextShapeData.h>
#include <KoShape.h>

class QPainter;
class KoViewConverter;
class KWTextFrameSet;
class MockTextShape;
class QTextDocument;
class QTextLayout;
class KWTextDocumentLayout;

class TestDocumentLayout : public QObject {
    Q_OBJECT
public:
    TestDocumentLayout() {}

private slots:
    void initTestCase();
    void cleanupTestCase();

    /// Test the hittest of KWTextDocumentLayout
    void testHitTest();

    /// Test breaking lines based on the width of the shape.
    void testLineBreaking();
    /// Test breaking lines for frames with different widths.
    void testMultiFrameLineBreaking();
    /// Tests incrementing Y pos based on the font size
    void testBasicLineSpacing();

// Block styles
    void testMargins();
    void testTextIndent();
    void testTextAlignments();
    // page breaks
    // tabs
    /// Test fo:line-height, style:lineheight-at-least and style:line-spacing properties (15.5.1)
    void testLineSpacing();

    //etc

private:
    void initForNewTest(const QString &initText = QString());

private:
    QApplication *m_app;

    KWTextFrameSet *frameSet;
    MockTextShape *shape1;
    QTextDocument *doc;
    KWTextDocumentLayout *layout;
    QTextLayout *blockLayout;
    QString loremIpsum;
};

class MockTextShape : public KoShape {
  public:
    MockTextShape() {
        setUserData(new KoTextShapeData());
    }
    void paint(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
};

#endif
