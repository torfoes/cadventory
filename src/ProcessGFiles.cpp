#include "ProcessGFiles.h"
#include <brlcad/ged.h>
#include <QDebug>
#include <iostream>

ProcessGFiles::ProcessGFiles(Model* model)
    : model(model)
{
}

void ProcessGFiles::processGFile(const ModelData& modelData)
{
    // Convert the file path to a C-style string
    const char* db_filename = modelData.file_path.c_str();

    // Open the BRL-CAD database
    struct ged* gedp = ged_open("db", db_filename, 0);
    if (gedp == GED_NULL) {
        std::cerr << "Error: Unable to open BRL-CAD database: " << db_filename << std::endl;
        return;
    }

    // Make a copy of modelData to modify
    ModelData updatedModelData = modelData;
    updatedModelData.is_processed = true;

    // Extract the title from the database
    extractTitle(updatedModelData, gedp);
    qDebug() << "[ProcessGFiles::processGFile] Title extracted:" << QString::fromStdString(updatedModelData.title);

    if (!model->updateModel(updatedModelData.id, updatedModelData)) {
        std::cerr << "[ProcessGFiles::processGFile] Error updating model in database." << std::endl;
        qDebug() << "[ProcessGFiles::processGFile] Error updating model in database for ID:" << updatedModelData.id;
        ged_close(gedp);
        return;
    } else {
        qDebug() << "[ProcessGFiles::processGFile] Successfully updated model with ID:" << updatedModelData.id;
    }

    // Extract and store objects
    extractObjects(updatedModelData, gedp);

    // TODO: Generate thumbnail (if necessary) using updatedModelData.previews_folder

    // Close the BRL-CAD database
    ged_close(gedp);
    qDebug() << "[ProcessGFiles::processGFile] Completed processGFile for path:" << QString::fromStdString(updatedModelData.file_path);
}

void ProcessGFiles::extractTitle(ModelData& modelData, struct ged* gedp)
{
    if (gedp && gedp->dbip && gedp->dbip->dbi_title) {
        std::string title(gedp->dbip->dbi_title);
        modelData.title = title;
        qDebug() << "[ProcessGFiles::extractTitle] Database title found:" << QString::fromStdString(title);
    } else {
        modelData.title = "(Untitled)";
        qDebug() << "[ProcessGFiles::extractTitle] No title found in database. Using '(Untitled)'";
    }
}

void ProcessGFiles::extractObjects(ModelData& modelData, struct ged* gedp)
{
    qDebug() << "[ProcessGFiles::extractObjects] Started for model ID:" << modelData.id;

    // Ensure gedp is valid
    if (!gedp || !gedp->dbip) {
        std::cerr << "[ProcessGFiles::extractObjects] Invalid ged pointer." << std::endl;
        qDebug() << "[ProcessGFiles::extractObjects] Invalid ged pointer. Cannot process objects.";
        return;
    }

    // Initialize the directory pointer to list top-level objects
    struct directory **dir = nullptr;
    qDebug() << "[ProcessGFiles::extractObjects] Listing top-level objects from the database.";
    size_t dir_count = db_ls(gedp->dbip, DB_LS_TOPS, nullptr, &dir);
    if (dir_count == 0) {
        std::cerr << "[ProcessGFiles::extractObjects] No objects found in database." << std::endl;
        qDebug() << "[ProcessGFiles::extractObjects] No objects found in database for model ID:" << modelData.id;
        return;
    }

    qDebug() << "[ProcessGFiles::extractObjects] Number of top-level objects found:" << dir_count;

    // Iterate over the directory entries for top-level objects
    for (size_t i = 0; i < dir_count; ++i) {
        std::string object_name(dir[i]->d_namep);
        qDebug() << "[ProcessGFiles::extractObjects] Found top-level object name:" << QString::fromStdString(object_name);

        // Create ObjectData for the top-level object
        ObjectData topLevelObjData;
        // In Model.h, ObjectData fields: int object_id, int model_id, ...
        // We'll use "object_id" as the DB ID field, which we fill after insertion
        topLevelObjData.model_id = modelData.id;
        topLevelObjData.name = object_name;
        topLevelObjData.parent_object_id = -1; // -1 indicates no parent
        topLevelObjData.is_selected = false;

        // Insert the top-level object into the database
        qDebug() << "[ProcessGFiles::extractObjects] Inserting top-level object - "
                 << "Model ID:" << topLevelObjData.model_id
                 << ", Name:" << QString::fromStdString(topLevelObjData.name)
                 << ", Parent Object ID:" << topLevelObjData.parent_object_id
                 << ", is_selected:" << topLevelObjData.is_selected;

        int insertedTopLevelObjectId = model->insertObject(topLevelObjData);
        if (insertedTopLevelObjectId == -1) {
            qDebug() << "[ProcessGFiles::extractObjects] Failed to insert top-level object:"
                     << QString::fromStdString(topLevelObjData.name)
                     << "for model ID:" << topLevelObjData.model_id;
            continue; // Move on to the next object
        } else {
            topLevelObjData.object_id = insertedTopLevelObjectId;
            qDebug() << "[ProcessGFiles::extractObjects] Successfully inserted top-level object:"
                     << QString::fromStdString(topLevelObjData.name)
                     << "with ID:" << insertedTopLevelObjectId << "for model ID:" << topLevelObjData.model_id;
        }

        // If this top-level object is a combination, retrieve and insert its children
        if (dir[i]->d_flags & RT_DIR_COMB) {
            qDebug() << "[ProcessGFiles::extractObjects] Object" << QString::fromStdString(object_name) << "is a combination. Retrieving children.";
            insertChildObjects(modelData, gedp, topLevelObjData);
        } else {
            qDebug() << "[ProcessGFiles::extractObjects] Object" << QString::fromStdString(object_name) << "is a primitive. No child objects to insert.";
        }
    }

    // Free the directory list for top-level objects
    bu_free(dir, "free directory list");
    qDebug() << "[ProcessGFiles::extractObjects] Completed for model ID:" << modelData.id;
}

void ProcessGFiles::insertChildObjects(ModelData& modelData, struct ged* gedp, const ObjectData& parentObjData)
{
    qDebug() << "[ProcessGFiles::insertChildObjects] Started for parent object ID:" << parentObjData.object_id << "Name:" << QString::fromStdString(parentObjData.name);

    // Retrieve the directory pointer for the parent object
    struct directory *parent_dir = db_lookup(gedp->dbip, parentObjData.name.c_str(), LOOKUP_QUIET);
    if (!parent_dir) {
        qDebug() << "[ProcessGFiles::insertChildObjects] Parent object" << QString::fromStdString(parentObjData.name) << "not found in database.";
        return;
    }

    // Check if the parent object is a combination
    if (!(parent_dir->d_flags & RT_DIR_COMB)) {
        qDebug() << "[ProcessGFiles::insertChildObjects] Parent object" << QString::fromStdString(parentObjData.name) << "is not a combination. No children to insert.";
        return;
    }

    // Retrieve the combination record
    struct rt_db_internal intern;
    struct rt_comb_internal *comb;
    if (rt_db_get_internal(&intern, parent_dir, gedp->dbip, nullptr, &rt_uniresource) < 0) {
        qDebug() << "[ProcessGFiles::insertChildObjects] Error retrieving internal representation for object" << QString::fromStdString(parentObjData.name);
        return;
    }

    comb = static_cast<struct rt_comb_internal*>(intern.idb_ptr);

    if (!comb->tree) {
        qDebug() << "[ProcessGFiles::insertChildObjects] Combination" << QString::fromStdString(parentObjData.name) << "has no children.";
        rt_db_free_internal(&intern);
        return;
    }

    // Retrieve child objects
    std::vector<std::string> children;
    db_tree_list_comb_children(comb->tree, children);

    qDebug() << "[ProcessGFiles::insertChildObjects] Number of children found for object" << QString::fromStdString(parentObjData.name) << ":" << children.size();

    // Insert each child object into the database
    for (const auto& child_name : children) {
        qDebug() << "[ProcessGFiles::insertChildObjects] Processing child object name:" << QString::fromStdString(child_name);

        // Lookup the child's directory entry
        struct directory *child_dir = db_lookup(gedp->dbip, child_name.c_str(), LOOKUP_QUIET);
        if (!child_dir) {
            qDebug() << "[ProcessGFiles::insertChildObjects] Child object" << QString::fromStdString(child_name) << "not found in database.";
            continue;
        }

        // Create ObjectData for the child
        ObjectData childObjData;
        childObjData.model_id = modelData.id;
        childObjData.name = child_name;
        childObjData.parent_object_id = parentObjData.object_id; // The parent's object ID
        childObjData.is_selected = false;

        qDebug() << "[ProcessGFiles::insertChildObjects] Inserting child object -"
                 << "Model ID:" << childObjData.model_id
                 << ", Name:" << QString::fromStdString(childObjData.name)
                 << ", Parent Object ID:" << childObjData.parent_object_id
                 << ", is_selected:" << childObjData.is_selected;

        int childInsertedObjectId = model->insertObject(childObjData);
        if (childInsertedObjectId == -1) {
            qDebug() << "[ProcessGFiles::insertChildObjects] Failed to insert child object:"
                     << QString::fromStdString(childObjData.name)
                     << "for parent object ID:" << parentObjData.object_id << "model ID:" << childObjData.model_id;
        } else {
            childObjData.object_id = childInsertedObjectId;
            qDebug() << "[ProcessGFiles::insertChildObjects] Successfully inserted child object:"
                     << QString::fromStdString(childObjData.name)
                     << "with ID:" << childInsertedObjectId << "for parent object ID:" << parentObjData.object_id << "model ID:" << childObjData.model_id;
        }
    }

    // Free the internal representation of the combination
    rt_db_free_internal(&intern);

    qDebug() << "[ProcessGFiles::insertChildObjects] Completed for parent object ID:" << parentObjData.object_id << "Name:" << QString::fromStdString(parentObjData.name);
}

void db_tree_list_comb_children(const union tree *tree, std::vector<std::string>& children) {
    if (!tree) return;

    switch (tree->tr_op) {
    case OP_UNION:
    case OP_INTERSECT:
    case OP_SUBTRACT:
    case OP_XOR:
        db_tree_list_comb_children(tree->tr_b.tb_left, children);
        db_tree_list_comb_children(tree->tr_b.tb_right, children);
        break;
    case OP_DB_LEAF:
        if (tree->tr_l.tl_name) {
            children.push_back(tree->tr_l.tl_name);
        }
        break;
    default:
        break;
    }
}
