#ifndef MODELVIEW_H
#define MODELVIEW_H

#include <QDialog>

#include "ui_modelview.h"
#include "Model.h"

class Model;

class ModelView : public QDialog {
  Q_OBJECT

 public:
  explicit ModelView(int modelId, Model* model, QWidget* parent = nullptr);
  ~ModelView();

 private:
  Ui::ModelView ui;
  int modelId;
  ModelData currModel;
  Model* model;
};

#endif  // modelview_H
