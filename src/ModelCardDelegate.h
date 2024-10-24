#ifndef MODELCARDDELEGATE_H
#define MODELCARDDELEGATE_H

#include <QStyledItemDelegate>
#include <QPixmap>

class ModelCardDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit ModelCardDelegate(QObject* parent = nullptr);

    // Override paint and sizeHint
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

    // Handle interactions
    bool editorEvent(QEvent* event, QAbstractItemModel* model,
                     const QStyleOptionViewItem& option, const QModelIndex& index) override;

signals:
    void settingsClicked(int modelId);

private:
    // Helper methods for component rectangles
    QRect previewRect(const QStyleOptionViewItem& option) const;
    QRect textRect(const QStyleOptionViewItem& option) const;
    QRect settingsIconRect(const QStyleOptionViewItem& option) const;

    // Cache the settings icon
    QPixmap settingsIcon;
};

#endif // MODELCARDDELEGATE_H
