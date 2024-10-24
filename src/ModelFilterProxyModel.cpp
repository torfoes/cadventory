// ModelFilterProxyModel.cpp

#include "ModelFilterProxyModel.h"
#include "Model.h"
#include <QRegularExpression>

ModelFilterProxyModel::ModelFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent) {
}

bool ModelFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    QVariant data = sourceModel()->data(index, filterRole());
    QString dataString;

    if (data.type() == QVariant::Bool) {
        dataString = data.toBool() ? "1" : "0";
    } else {
        dataString = data.toString();
    }

    if (filterRegularExpression().pattern().isEmpty()) {
        return true;
    }

    return dataString.contains(filterRegularExpression());
}
