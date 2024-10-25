#ifndef MODELCARDDELEGATE_H
#define MODELCARDDELEGATE_H

#include <QStyledItemDelegate>
#include <QPixmap>

class ModelCardDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit ModelCardDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model,
                     const QStyleOptionViewItem& option, const QModelIndex& index) override;

signals:
    void settingsClicked(int modelId) const;

private:
    // Helper methods to calculate component rectangles
    QRect previewRect(const QStyleOptionViewItem& option) const;
    QRect textRect(const QStyleOptionViewItem& option) const;
    QRect settingsIconRect(const QStyleOptionViewItem& option) const;

    QPixmap settingsIcon;
};

#endif // MODELCARDDELEGATE_H
