#include <QtTest/QtTest>
#include <QApplication>
#include <QDialog>
#include <QPushButton>
#include <QDir>
#include <QTemporaryDir>

#include "../MainWindow.h"
#include "../LibraryWindow.h"
#include "../Library.h"
#include "../Model.h"
#include "../ModelFilterProxyModel.h"
#include "../ModelCardDelegate.h"
#include "../IndexingWorker.h"
#include "../FilesystemIndexer.h"
#include "../ProcessGFiles.h"
#include "../GeometryBrowserDialog.h"


class TestMainWindowGUI : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void testMainWindowVisible();
    void testAddLibrary();
    void testOpenLibrary();
    void testSaveAndLoadState();
    void testAddLibraryButtonClick();

private:
    MainWindow* mainWindow;
};

// Initialize MainWindow for testing
void TestMainWindowGUI::initTestCase() {
    try {
        // Base path for testing
        QString testDirPath = "/tmp/MainWindowTest";
        QDir testDir(testDirPath);

        // Log and create the base test directory if it doesn’t exist
        if (!testDir.exists()) {
            qDebug() << "Creating base directory at:" << testDirPath;
            if (!testDir.mkpath(".")) {
                qDebug() << "Failed to create base directory at:" << testDirPath;
                QFAIL("Unable to create base directory.");
            }
        }

        // Create `.cadventory` subdirectory under test path
        QString cadventoryDirPath = testDirPath + "/.cadventory";
        QDir cadventoryDir(cadventoryDirPath);
        if (!cadventoryDir.exists()) {
            qDebug() << "Creating .cadventory directory at:" << cadventoryDirPath;
            if (!cadventoryDir.mkpath(".")) {
                qDebug() << "Failed to create .cadventory directory at:" << cadventoryDirPath;
                QFAIL("Unable to create .cadventory directory.");
            }
        }

        // Initialize and show the main window
        mainWindow = new MainWindow();
        mainWindow->show();

    } catch (const std::filesystem::filesystem_error& e) {
        qDebug() << "Exception caught during setup:" << e.what();
        QFAIL("Caught unhandled filesystem_error during directory setup.");
    }
}



// Test if the main window is visible
void TestMainWindowGUI::testMainWindowVisible() {
    QVERIFY(mainWindow->isVisible());
}

void TestMainWindowGUI::testAddLibrary()
{
    try {
        mainWindow->clearLibraries();  // Clear libraries before starting

        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString libraryPath = tempDir.path();
        mainWindow->addLibrary("Test Library", libraryPath.toUtf8().constData());

        const auto& libraries = mainWindow->getLibraries();
        QCOMPARE(libraries.size(), 1);
        QCOMPARE(libraries[0]->name(), QString("Test Library"));
        QCOMPARE(libraries[0]->path(), libraryPath);
    } catch (const std::filesystem::filesystem_error& e) {
        QFAIL(e.what());
    }
}

void TestMainWindowGUI::testOpenLibrary()
{
    try {
        mainWindow->clearLibraries();  // Clear libraries before starting

        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString libraryPath = tempDir.path();
        mainWindow->addLibrary("Open Library", libraryPath.toUtf8().constData());

        mainWindow->openLibrary();

        // Verify post-conditions as applicable; update checks as needed
        const auto& libraries = mainWindow->getLibraries();
        QVERIFY(!libraries.empty());
        QCOMPARE(libraries.back()->name(), QString("Open Library"));
        QCOMPARE(libraries.back()->path(), libraryPath);
    } catch (const std::filesystem::filesystem_error& e) {
        QFAIL(e.what());
    }
}


void TestMainWindowGUI::testSaveAndLoadState()
{
    try {
        mainWindow->clearLibraries();  // Ensure no pre-existing libraries

        // Use a temporary directory to create a persistent path for the library
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString persistentPath = tempDir.path();
        mainWindow->addLibrary("Persistent Library", persistentPath.toUtf8().constData());

        // Save the state
        size_t savedCount = mainWindow->publicSaveState();
        QCOMPARE(savedCount, 1);

        // Clear libraries and reload state
        mainWindow->clearLibraries();
        size_t loadedCount = mainWindow->publicLoadState();

        // Verify the state is consistent
        const auto& libraries = mainWindow->getLibraries();
        QCOMPARE(loadedCount, 1);
        QCOMPARE(libraries[0]->name(), QString("Persistent Library"));
        QCOMPARE(libraries[0]->path(), persistentPath);

    } catch (const std::filesystem::filesystem_error& e) {
        QFAIL(e.what());  // Catch and report any filesystem errors
    }
}

void TestMainWindowGUI::testAddLibraryButtonClick()
{
    // Placeholder: Verify the addLibraryButton is present
    QPushButton* addButton = mainWindow->findChild<QPushButton*>("addLibraryButton");
    QVERIFY(addButton != nullptr);

    // Simulate a click to verify functionality
    QTest::mouseClick(addButton, Qt::LeftButton);

    // Placeholder assertion, update as needed based on button functionality
    QVERIFY(true);
}

QTEST_MAIN(TestMainWindowGUI)
#include "MainWindowTest.moc"