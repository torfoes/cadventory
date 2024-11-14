#include "ProcessGFiles.h"
#include <brlcad/ged.h>
#include <iostream>

ProcessGFiles::ProcessGFiles(Model* model, bool debug)
    : model(model)
{
}

void ProcessGFiles::processGFile(const std::filesystem::path& file_path,
                                 const std::string& previews_folder,
                                 const std::string& library_name)
{
    // Convert file_path to a C-style string
    const char* db_filename = file_path.c_str();

    // Open the BRL-CAD database
    struct ged* gedp = ged_open("db", db_filename, 0);

    if (gedp == GED_NULL) {
        std::cerr << "Error: Unable to open BRL-CAD database: " << db_filename << std::endl;
        return;
    }

    // Create a ModelData object to store the extracted information
    ModelData modelData;
    modelData.file_path = file_path.string();
    modelData.library_name = library_name;
    modelData.short_name = file_path.filename().string();

    // Extract the title from the database
    extractTitle(modelData, gedp);
    qDebug() << "Title extracted:" << QString::fromStdString(modelData.title);


    // Update the model with the new modelData
    // model->insertOrUpdateModel(modelData);

    // Close the BRL-CAD database
    ged_close(gedp);
}

void ProcessGFiles::extractTitle(ModelData& modelData, struct ged* gedp)
{
    if (gedp && gedp->dbip && gedp->dbip->dbi_title) {
        std::string title(gedp->dbip->dbi_title);
        modelData.title = title;
    } else {
        modelData.title = "(Untitled)";
    }
}
