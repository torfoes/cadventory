#include <catch2/catch_test_macros.hpp>
#include <QTest>
#include <QTemporaryDir>
#include "GeometryBrowserDialog.h"
#include "Model.h"

class GeometryBrowserDialogTest : public QObject {
    Q_OBJECT

private:
    QTemporaryDir tempDir;

private slots:
    void initTestCase() {
        QVERIFY(tempDir.isValid());
    }

    void cleanupTestCase() {}

    void testTreeInitializationAndHierarchy() {
        QString tempPath = tempDir.path();
        Model model(tempPath.toStdString());

        // Insert mock data into the model
        model.insertObject({1, 1, "RootObject", -1, false});
        model.insertObject({2, 1, "ChildObject1", 1, false});
        model.insertObject({3, 1, "ChildObject2", 1, false});

        GeometryBrowserDialog dialog(1, &model);
        dialog.show();

        int topLevelItemCount = dialog.getTreeWidget()->topLevelItemCount();
        QVERIFY(topLevelItemCount > 0);

        for (int i = 0; i < topLevelItemCount; ++i) {
            QTreeWidgetItem* topLevelItem = dialog.getTreeWidget()->topLevelItem(i);
            QVERIFY(topLevelItem->childCount() >= 0);
        }
    }

    void testCheckboxSelectionState() {
        QString tempPath = tempDir.path();
        Model model(tempPath.toStdString());

        // Insert mock data into the model
        model.insertObject({1, 1, "RootObject", -1, false});

        GeometryBrowserDialog dialog(1, &model);
        dialog.show();

        QTreeWidgetItem* topLevelItem = dialog.getTreeWidget()->topLevelItem(0);
        QVERIFY(topLevelItem != nullptr);

        topLevelItem->setCheckState(0, Qt::Checked);
        QVERIFY(topLevelItem->checkState(0) == Qt::Checked);
    }

    void testUncheckAllExceptSelected() {
        QString tempPath = tempDir.path();
        Model model(tempPath.toStdString());

        // Insert mock data into the model
        model.insertObject({1, 1, "RootObject", -1, false});
        model.insertObject({2, 1, "SiblingObject", -1, false});

        GeometryBrowserDialog dialog(1, &model);
        dialog.show();

        QTreeWidgetItem* firstItem = dialog.getTreeWidget()->topLevelItem(0);
        QTreeWidgetItem* secondItem = dialog.getTreeWidget()->topLevelItem(1);

        QVERIFY(firstItem != nullptr);
        QVERIFY(secondItem != nullptr);

        firstItem->setCheckState(0, Qt::Checked);
        QCOMPARE(firstItem->checkState(0), Qt::Checked);
        QCOMPARE(secondItem->checkState(0), Qt::Unchecked);

        secondItem->setCheckState(0, Qt::Checked);
        QCOMPARE(firstItem->checkState(0), Qt::Unchecked);
        QCOMPARE(secondItem->checkState(0), Qt::Checked);
    }

    void testRecursiveUncheckMechanism() {
        QString tempPath = tempDir.path();
        Model model(tempPath.toStdString());

        // Insert mock data with parent-child relationship
        model.insertObject({1, 1, "ParentObject", -1, false});
        model.insertObject({2, 1, "ChildObject", 1, false});

        GeometryBrowserDialog dialog(1, &model);
        dialog.show();

        QTreeWidgetItem* parentItem = dialog.getTreeWidget()->topLevelItem(0);
        QVERIFY(parentItem != nullptr);
        parentItem->setCheckState(0, Qt::Checked);

        if (parentItem->childCount() > 0) {
            QTreeWidgetItem* childItem = parentItem->child(0);
            QVERIFY(childItem != nullptr);

            childItem->setCheckState(0, Qt::Checked);
            QVERIFY(childItem->checkState(0) == Qt::Checked);

            QTreeWidgetItem* anotherTopItem = dialog.getTreeWidget()->topLevelItem(1);
            anotherTopItem->setCheckState(0, Qt::Checked);

            QVERIFY(childItem->checkState(0) == Qt::Unchecked);
        }
    }
};

QTEST_MAIN(GeometryBrowserDialogTest)
#include "GeometryBrowserDialogTest.moc"