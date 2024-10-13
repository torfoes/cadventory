#include "audit.h"


/*

    At the current moment, the auditing functionality is in regards to the presence of files.
    At a later date, I will extend this to the tagging functionality present with CADventory,
    whenever this feature is implemented.

    -- Anton 10/13


*/


Audit::Audit(QSqlDatabase& db, std::vector<Library*> libraries) {
    this->db = db;
    this->libraries = libraries;
    // calculate checksums of libraries
    std::vector<std::pair<std::string,std::string>> cur_checksums = checksumsAll();
    // peform routine
    checkFiles();

}

std::vector<std::pair<std::string,std::string>> Audit::checksumsAll(){
    // pair: (file, checksum)
    std::vector<std::pair<std::string, std::string>> to_ret;
    for(auto lib: libraries){
        std::cout<< "library: " << lib->name() << std::endl;
        // iterate through each file type
        std::vector<std::string> model_files = lib->getModels();
        std::vector<std::string> image_files = lib->getImages();
        std::vector<std::string> geometry_files = lib->getGeometry();
        std::vector<std::string> data_files = lib->getData();
        std::vector<std::string> document_files = lib->getDocuments();

        std::string lib_name = lib->name();
        for(std::string model_path: model_files){
            std::cout << "model_path: " << model_path << std::endl;
            std::string checksum = calculateCheckSum(model_path);
            // insertFileChecksum(model_path, checksum, lib_name);
            to_ret.push_back(std::pair<std::string,std::string>(model_path, checksum));
        }

        for(std::string img_path: image_files){
            std::cout << "img_path: " << img_path << std::endl;
            std::string checksum = calculateCheckSum(img_path);
            // insertFileChecksum(img_path, checksum, lib_name);
            to_ret.push_back(std::pair<std::string,std::string>(img_path, checksum));
        }

        for(std::string geom_path: geometry_files){
            std::cout << "geom_path: " << geom_path << std::endl;
            std::string checksum = calculateCheckSum(geom_path);
            // insertFileChecksum(geom_path, checksum, lib_name);
            to_ret.push_back(std::pair<std::string,std::string>(geom_path, checksum));
        }

        for(std::string data_path: data_files){
            std::cout << "geom_path: " << data_path << std::endl;
            std::string checksum = calculateCheckSum(data_path);
            // insertFileChecksum(data_path, checksum, lib_name);
            to_ret.push_back(std::pair<std::string,std::string>(data_path, checksum));
        }

        for(std::string doc_path: document_files){
            std::cout << "doc_path: " << doc_path << std::endl;
            std::string checksum = calculateCheckSum(doc_path);
            // insertFileChecksum(doc_path, checksum, lib_name);
            to_ret.push_back(std::pair<std::string,std::string>(doc_path, checksum));

        }

    }


    return to_ret;
}

void Audit::insertFileChecksum(std::string &path, std::string &checksum, std::string& lib_name){
    // insert into database
    QSqlQuery q;
    q.prepare("insert into file_checksums(filepath, checksum, library) values (?, ?, ?)");
    q.addBindValue(QString::fromStdString(path));
    q.addBindValue(QString::fromStdString(checksum));
    q.addBindValue(QString::fromStdString(lib_name));

    bool res = q.exec();
    if(res){
        std::cout << "worked!" << std::endl;
    }else{
        std::cout << "error: " << q.lastError().text().toStdString() << std::endl;
    }
    

}

std::string Audit::calculateCheckSum(std::string& file){

    QFile f(QString::fromStdString(file));
    if(f.open(QFile::ReadOnly)){
    QCryptographicHash hash(QCryptographicHash::Md5);
      if(hash.addData(&f)){
        return QString(hash.result().toHex(0)).toStdString();
      }
    }
    return "";
}



void Audit::checkFiles(){
    // generate list of new, modified, and removed files
    // if new -> insert into database
    // if removed -> check if in database and remove
    // if modified -> update entry in database
    //              -> if moved -> update file path
    //              -> update checksum
    // display modifications, prompt user to re-index new files, and other things (ask sean).





}
