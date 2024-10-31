#include "ModelView.h"
#include "Model.h"
#include "ui_modelview.h"


ModelView::ModelView(int modelId, Model* model, QWidget* parent)
    : QDialog(parent), modelId(modelId), model(model) {

    ui.setupUi(this);
    ModelData modelData = model->getModelById(modelId);

    ui.modelName->setText(QString::fromStdString(modelData.short_name));
    ui.libraryName->setText(QString::fromStdString(modelData.library_name));

    
}

ModelView::~ModelView() {
    qDebug() << "ModelView destructor called";
}