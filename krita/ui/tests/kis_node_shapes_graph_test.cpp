/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_node_shapes_graph_test.h"

#include <qtest_kde.h>

#include "kis_node_shapes_graph.h"
#include "node_shapes_utils.h"


/**
 * node0
 * +--node1
 *    +--node7
 *    +--node8
 * +--node2
 * +--node3
 * +--node4
 *    +--node9
 *    +--node10
 *    +--node11
 * +--node5
 * +--node6
 */

void KisNodeShapesGraphTest::init()
{
    m_shapesGraph = new KisNodeShapesGraph();
    KisNodeSP rootNode = nodeFromId(0);
    m_shapesGraph->addNode(rootNode, 0, 0);
    m_rootDummy = m_shapesGraph->nodeToDummy(rootNode);


    KisNodeSP parent;

    parent = rootNode;
    for(int i = 6; i >= 1; i--) {
        m_shapesGraph->addNode(nodeFromId(i), parent, 0);
    }

    parent = findNodeById(m_rootDummy, 1);
    Q_ASSERT(parent);
    for(int i = 8; i >= 7; i--) {
        m_shapesGraph->addNode(nodeFromId(i), parent, 0);
    }

    parent = findNodeById(m_rootDummy, 4);
    Q_ASSERT(parent);
    for(int i = 11; i >= 9; i--) {
        m_shapesGraph->addNode(nodeFromId(i), parent, 0);
    }

    QString realGraph = collectGraphPattern(m_rootDummy);
    QString expectedGraph = "0 1 7 8 2 3 4 9 10 11 5 6";

    QCOMPARE(realGraph, expectedGraph);
}

void KisNodeShapesGraphTest::cleanup()
{
    delete m_shapesGraph;
}

void KisNodeShapesGraphTest::testShapeChildren()
{
    KisNodeShape *parent = m_shapesGraph->nodeToShape(findNodeById(m_rootDummy, 4));
    QList<KisNodeShape*> expectedChildren;

    for(int i = 9; i <= 11; i++) {
        expectedChildren.append(m_shapesGraph->nodeToShape(findNodeById(m_rootDummy, i)));
    }

    QList<KoShape*> realChildren = parent->shapes();

    foreach(KoShape *shape, realChildren) {
        KisNodeShape *nodeShape = dynamic_cast<KisNodeShape*>(shape);

        QVERIFY(expectedChildren.contains(nodeShape));
        expectedChildren.removeOne(nodeShape);
    }

    QVERIFY(expectedChildren.isEmpty());
}

void KisNodeShapesGraphTest::testInsert()
{
    KisNodeSP parent = findNodeById(m_rootDummy, 4);
    KisNodeSP aboveThis = findNodeById(m_rootDummy, 10);
    KisNodeSP node = nodeFromId(13);

    KisNodeShape *addedShape =
        m_shapesGraph->addNode(node, parent, aboveThis);

    QString realGraph = collectGraphPattern(m_rootDummy);
    QString expectedGraph = "0 1 7 8 2 3 4 9 10 13 11 5 6";
    QCOMPARE(realGraph, expectedGraph);

    KisNodeShape *nodeShape = m_shapesGraph->nodeToShape(node);
    KisNodeShape *parentShape = m_shapesGraph->nodeToShape(parent);
    QCOMPARE(addedShape, nodeShape);
    QCOMPARE(((KoShape*)nodeShape)->parent(), parentShape);
}

void KisNodeShapesGraphTest::testRemove()
{
    KisNodeSP parent = findNodeById(m_rootDummy, 4);
    KisNodeSP node = findNodeById(m_rootDummy, 10);

    KisNodeShape *parentShape = m_shapesGraph->nodeToShape(parent);

    QCOMPARE(parentShape->shapeCount(), 3);
    m_shapesGraph->removeNode(node);
    QCOMPARE(parentShape->shapeCount(), 2);

    QString realGraph = collectGraphPattern(m_rootDummy);
    QString expectedGraph = "0 1 7 8 2 3 4 9 11 5 6";
    QCOMPARE(realGraph, expectedGraph);
}

void KisNodeShapesGraphTest::testRemoveRootNode()
{
    KisNodeSP root = findNodeById(m_rootDummy, 0);
    m_rootDummy = 0;

    m_shapesGraph->removeNode(root);
    QCOMPARE(m_shapesGraph->shapesCount(), 0);
}

QTEST_KDEMAIN(KisNodeShapesGraphTest, GUI)
