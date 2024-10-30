// #include <QtTest/QtTest>
// #include <QDialog>

// #include "../MainWindow.h"

// class TestMainWindowGUI : public QObject {
//     Q_OBJECT

// private slots:
//     void initTestCase();
//     void testMainWindowVisible();
//     void testAddLibraryButton();
//     //void testOpenLibrary();

// private:
//     MainWindow* mainWindow;
// };

// // Initialize MainWindow for testing
// void TestMainWindowGUI::initTestCase() {
//     mainWindow = new MainWindow();
//     mainWindow->show();
// }

// // Test if the main window is visible
// void TestMainWindowGUI::testMainWindowVisible() {
//     QVERIFY(mainWindow->isVisible());
// }

// // Test if the "Add Library" button opens the correct dialog
// void TestMainWindowGUI::testAddLibraryButton() {
//     // Assuming the "Add Library" button has an accessible name or object name
//     QPushButton* addLibraryButton = mainWindow->findChild<QPushButton*>("addLibraryButton");
//     QVERIFY(addLibraryButton != nullptr);

//     // Simulate a click on the "Add Library" button
//     QSignalSpy spy(addLibraryButton, &QPushButton::clicked);
//     QTest::mouseClick(addLibraryButton, Qt::LeftButton);
//     QVERIFY(spy.count() == 1);

//     // Check if the dialog or expected widget appears
//     // Assuming a dialog appears named "AddLibraryDialog"
//     QDialog* addLibraryDialog = mainWindow->findChild<QDialog*>("AddLibraryDialog");
//     QVERIFY(addLibraryDialog != nullptr);
//     QVERIFY(addLibraryDialog->isVisible());
// }

// // Test if a library tile can be clicked to open the library
// // void TestMainWindowGUI::testOpenLibrary() {
// //     // Assuming there’s a QWidget representing library tiles with object names
// //     QWidget* libraryTile = mainWindow->findChild<QWidget*>("LibraryTile_0"); // Example tile
// //     QVERIFY(libraryTile != nullptr);

// //     // Simulate clicking on the library tile
// //     QSignalSpy openSpy(mainWindow, &MainWindow::libraryOpened); // Assuming signal exists
// //     QTest::mouseClick(libraryTile, Qt::LeftButton);
// //     QVERIFY(openSpy.count() == 1);

// //     // Verify that the contents of the library loaded as expected
// //     // This can be more detailed based on what "open" means for your UI
// // }

// QTEST_MAIN(TestMainWindowGUI)
// #include "MainWindowTest.moc"
