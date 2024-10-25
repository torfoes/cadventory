#include "ModelCardDelegate.h"
#include "Model.h"
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QStyle>
#include <QIcon>

ModelCardDelegate::ModelCardDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
    settingsIcon = QIcon(":/assets/icons/settings_icon.png").pixmap(24, 24);
}

void ModelCardDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const {
    painter->save();

    // Retrieve data
    QPixmap thumbnail = index.data(Model::ThumbnailRole).value<QPixmap>();
    QString shortName = index.data(Model::ShortNameRole).toString();
    QString title = index.data(Model::TitleRole).toString();
    QString author = index.data(Model::AuthorRole).toString();
    int modelId = index.data(Model::IdRole).toInt();

    // draw background
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else {
        painter->fillRect(option.rect, option.palette.window());
    }

    // calculate rectangles
    QRect previewR = previewRect(option);
    QRect textR = textRect(option);
    QRect settingsR = settingsIconRect(option);

    if (!thumbnail.isNull()) {
        painter->drawPixmap(previewR, thumbnail.scaled(previewR.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    } else {
        // placeholder if no thumbnail
        painter->fillRect(previewR, Qt::lightGray);
        painter->drawText(previewR, Qt::AlignCenter, "No Image");
    }

    // text
    painter->setPen(option.palette.text().color());
    QFont boldFont = option.font;
    boldFont.setBold(true);
    painter->setFont(boldFont);
    painter->drawText(textR, Qt::AlignLeft | Qt::AlignTop, shortName);

    painter->setFont(option.font);
    painter->drawText(textR.adjusted(0, 20, 0, 0), Qt::AlignLeft | Qt::AlignTop, title);
    painter->drawText(textR.adjusted(0, 40, 0, 0), Qt::AlignLeft | Qt::AlignTop, author);

    // Draw settings icon
    painter->drawPixmap(settingsR.topLeft(), settingsIcon);

    painter->restore();
}

QSize ModelCardDelegate::sizeHint(const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(300, 100);
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

        // handle selection
        if (option.rect.contains(pos)) {
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

// Helper methods to calculate component rectangles
QRect ModelCardDelegate::previewRect(const QStyleOptionViewItem& option) const {
    int margin = 10;
    int imageSize = option.rect.height() - 2 * margin;
    return QRect(option.rect.left() + margin, option.rect.top() + margin, imageSize, imageSize);
}

QRect ModelCardDelegate::textRect(const QStyleOptionViewItem& option) const {
    int margin = 10;
    int imageSize = option.rect.height() - 2 * margin;
    int textX = option.rect.left() + margin + imageSize + margin;
    int textWidth = option.rect.width() - imageSize - 3 * margin - settingsIcon.width();
    return QRect(textX, option.rect.top() + margin, textWidth, option.rect.height() - 2 * margin);
}

QRect ModelCardDelegate::settingsIconRect(const QStyleOptionViewItem& option) const {
    int margin = 10;
    int iconSize = 24;
    int x = option.rect.right() - margin - iconSize;
    int y = option.rect.top() + margin;
    return QRect(x, y, iconSize, iconSize);
}

