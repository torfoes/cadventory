#include "ModelCardDelegate.h"
#include "Model.h"
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QStyle>
#include <QIcon>

ModelCardDelegate::ModelCardDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
    // Load the settings icon
    settingsIcon = QIcon::fromTheme("settings", QIcon(":/icons/settings.png")).pixmap(24, 24);
}

void ModelCardDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const {
    painter->save();

    // Retrieve data
    QPixmap thumbnail = index.data(Model::ThumbnailRole).value<QPixmap>();
    QString shortName = index.data(Model::ShortNameRole).toString();
    QString title = index.data(Model::TitleRole).toString();
    int modelId = index.data(Model::IdRole).toInt();

    // Draw background
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else {
        painter->fillRect(option.rect, option.palette.window());
    }

    // Draw preview image
    QRect previewR = previewRect(option);
    if (!thumbnail.isNull()) {
        painter->drawPixmap(previewR, thumbnail.scaled(previewR.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    } else {
        // Placeholder if no thumbnail
        painter->fillRect(previewR, Qt::lightGray);
        painter->drawText(previewR, Qt::AlignCenter, "No Image");
    }

    // Draw text
    QRect textR = textRect(option);
    painter->setPen(option.palette.text().color());
    painter->drawText(textR, Qt::AlignLeft | Qt::AlignTop, shortName);
    painter->drawText(textR.adjusted(0, 20, 0, 0), Qt::AlignLeft | Qt::AlignTop, title);

    // Draw settings icon
    QRect settingsR = settingsIconRect(option);
    painter->drawPixmap(settingsR.topLeft(), settingsIcon);

    painter->restore();
}

QSize ModelCardDelegate::sizeHint(const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(200, 220); // Adjust as needed
}

bool ModelCardDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                    const QStyleOptionViewItem& option, const QModelIndex& index) {
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint pos = mouseEvent->pos();

        // Check if the settings icon area was clicked
        if (settingsIconRect(option).contains(pos)) {
            int modelId = index.data(Model::IdRole).toInt();
            emit settingsClicked(modelId);
            return true;
        }

        // Handle selection (if not already handled by the view)
        if (option.rect.contains(pos)) {
            bool isSelected = index.data(Qt::CheckStateRole).toBool();
            model->setData(index, !isSelected, Qt::CheckStateRole);
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

// Helper methods to calculate component rectangles
QRect ModelCardDelegate::previewRect(const QStyleOptionViewItem& option) const {
    int margin = 10;
    int size = option.rect.width() - 2 * margin;
    return QRect(option.rect.left() + margin, option.rect.top() + margin, size, size);
}

QRect ModelCardDelegate::textRect(const QStyleOptionViewItem& option) const {
    int margin = 10;
    int yOffset = option.rect.top() + option.rect.width() - margin + 5;
    int height = option.rect.height() - option.rect.width() - 2 * margin;
    return QRect(option.rect.left() + margin, yOffset, option.rect.width() - 2 * margin, height);
}

QRect ModelCardDelegate::settingsIconRect(const QStyleOptionViewItem& option) const {
    int iconSize = 24;
    int margin = 10;
    return QRect(option.rect.right() - iconSize - margin, option.rect.top() + margin, iconSize, iconSize);
}
