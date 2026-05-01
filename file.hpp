#pragma once

#include <fstream>
#include <string>

class FileManager {
    public:
        bool openFile(std::string& filename);
        void createFile(const std::string& name) const;
        void closeFile();
        void saveData(const std::string& data);
        std::string readData();
        void saveAs(std::string& newFilename, const std::string& data);

    private:
        std::fstream file;
        std::string defaultFileName = "file.json";
        std::string currentFileName;
};