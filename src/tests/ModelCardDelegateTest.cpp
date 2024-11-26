#include <catch2/catch_test_macros.hpp>
#include <QTest>
#include <QTemporaryDir>
#include <QStandardItemModel>
#include <QPainter>
#include <QSignalSpy>
#include "ModelCardDelegate.h"
#include "Model.h"

// Test class for the ModelCardDelegate component
class ModelCardDelegateTest : public QObject {
    Q_OBJECT

private slots:
    // Test painting logic for the delegate
    void testPaintLogic() {
        QStandardItemModel model(1, 1);
        QModelIndex index = model.index(0, 0);

        // Mock data for the index
        model.setData(index, QPixmap(100, 100), Model::ThumbnailRole);
        model.setData(index, QString("Short Name"), Model::ShortNameRole);
        model.setData(index, QString("Title"), Model::TitleRole);
        model.setData(index, QString("Author"), Model::AuthorRole);

        ModelCardDelegate delegate;

        // Initialize a valid QPixmap for the painter
        QPixmap pixmap(300, 100);
        pixmap.fill(Qt::white); // Ensure the pixmap is filled with a valid color

        QPainter painter(&pixmap);
        REQUIRE(painter.isActive()); // Verify that the QPainter is active

        QStyleOptionViewItem option;
        option.rect = QRect(0, 0, 300, 100);

        // Perform painting and handle exceptions
        try {
            delegate.paint(&painter, option, index);
            REQUIRE(true); // Pass if no exception is thrown
        } catch (const std::exception& e) {
            FAIL_CHECK(std::string("Exception during paint: ") + e.what());
        } catch (...) {
            FAIL_CHECK("Unknown exception during paint.");
        }
    }


    // Test size hint logic
    void testSizeHint() {
        QStandardItemModel model(1, 1);
        QModelIndex index = model.index(0, 0);

        ModelCardDelegate delegate;

        QStyleOptionViewItem option;
        QSize sizeHint = delegate.sizeHint(option, index);

        // Verify the size hint matches the expected value
        QCOMPARE(sizeHint.width(), 300);
        QCOMPARE(sizeHint.height(), 100);
    }

    // Test interaction with the editorEvent method
    void testEditorEvent() {
        QStandardItemModel model(1, 1);
        QModelIndex index = model.index(0, 0);

        // Mock data for the index
        model.setData(index, 123, Model::IdRole);

        ModelCardDelegate delegate;

        QMouseEvent mouseEvent(
            QEvent::MouseButtonRelease,
            QPointF(290, 20),   // Updated to use QPointF
            Qt::LeftButton,
            Qt::LeftButton,
            Qt::NoModifier
        );
        QStyleOptionViewItem option;
        option.rect = QRect(0, 0, 300, 100);

        // Create a spy for the signal
        QSignalSpy spy(&delegate, &ModelCardDelegate::modelViewClicked);

        // Trigger the editor event
        REQUIRE(delegate.editorEvent(&mouseEvent, &model, option, index));

        // Verify the signal was emitted with the correct data
        REQUIRE(spy.count() == 1);
        QCOMPARE(spy.takeFirst().at(0).toInt(), 123);
    }
};

// Main function for running the test cases
QTEST_MAIN(ModelCardDelegateTest)
#include "ModelCardDelegateTest.moc"