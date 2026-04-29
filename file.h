class FileManager {
    public:
        bool openFile(std::string& filename) {
            file.clear();
            file.open(filename, std::ios::in | std::ios::out | std::ios::app);
            if (!file.is_open()) {
                file.clear();
                file.open(defaultFileName, std::ios::in | std::ios::out | std::ios::app);
                std::cout << "Can't open/create the file: " << filename << std::endl;
                if (!file.is_open()) {
                    file.clear();
                    std::cout << "Can't open/create the file: " << defaultFileName << std::endl;
                    return false;
                }
                filename = defaultFileName;
            }
            std::cout << "The file is open/create: " << filename << std::endl;
            return true;
        }

        void closeFile() {
            if (file.is_open()) {
                file.close();
                std::cout << "The file is closed." << std::endl;
            }
            file.clear();
        }

        void saveData(const std::string& data) {
            if (file.is_open()) {
                file.clear();
                file << data << std::endl;
                std::cout << "The data has been written to the file." << std::endl;
            } else {
                std::cout << "The file is not open. Data cannot be written." << std::endl;
            }
        }

        std::string readData() {
            if (!file.is_open()) {
                return "";
            }
            file.clear();

            file.seekg(0, std::ios::end);
            size_t size = file.tellg();
            std::string data;
            data.resize(size);
            file.seekg(0, std::ios::beg);
            file.read(&data[0], size);
            return data;
        }

        void saveAs(std::string& newFilename, const std::string& data) {
            if (file.is_open()) {
                file.close();
            }
            if (!openFile(newFilename)) {
                return;
            }
            saveData(data);
        }

    private:
        std::fstream file;
        std::string defaultFileName = "file.json";
};