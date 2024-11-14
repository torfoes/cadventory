#ifndef PROCESSGFILES_H
#define PROCESSGFILES_H

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "Model.h"
#include <brlcad/rt/geom.h>

class ProcessGFiles {
public:
    explicit ProcessGFiles(Model* model);

    void processGFile(const ModelData& modelData);

private:
    void extractTitle(ModelData& modelData, struct ged* gedp);
    void extractObjects(ModelData& modelData, struct ged* gedp);
    void insertChildObjects(ModelData& modelData, struct ged* gedp, const ObjectData& parentObjData);

    Model* model;
};

void db_tree_list_comb_children(const union tree *tree, std::vector<std::string>& children);

#endif  // PROCESSGFILES_H
