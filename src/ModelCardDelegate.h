#ifndef MODELCARDDELEGATE_H
#define MODELCARDDELEGATE_H

#include <QStyledItemDelegate>
#include <QPixmap>

class ModelCardDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit ModelCardDelegate(QObject* parent = nullptr);

    // Overridden methods
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model,
                     const QStyleOptionViewItem& option, const QModelIndex& index) override;

signals:
    void geometryBrowserClicked(int modelId);  // Signal for the red rectangle

private:
    // Helper methods to calculate component rectangles
    QRect previewRect(const QStyleOptionViewItem& option) const;
    QRect textRect(const QStyleOptionViewItem& option) const;
    QRect redRectangleRect(const QStyleOptionViewItem& option) const;

    // No need for settingsIcon since we're using a red rectangle
};

#endif // MODELCARDDELEGATE_H
