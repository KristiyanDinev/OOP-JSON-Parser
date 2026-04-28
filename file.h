class FileManager {
    public:
        void openFile(std::string& filename) {
            file.open(filename);
            if (!file.is_open()) {
                filename = defaultFileName;
                file.open(filename);
            }
            std::cout << "The file is open: " << filename << std::endl;
        }

        void closeFile() {
            if (file.is_open()) {
                file.close();
                std::cout << "The file is closed." << std::endl;
            }
        }

        void saveData(const std::string& data) {
            if (file.is_open()) {
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
            std::string data;
            std::string line;
            while (std::getline(file, line)) {
                data += line;
            }
            return data;
        }

        void saveAs(const std::string& newFilename, const JsonManager& jsonManager) {
            if (file.is_open()) {
                file.close();
            }
            file.open(newFilename);

            file << jsonManager.prettyData() << std::endl;
            file.close();
            std::cout << "The data has been written to the file: " << newFilename << std::endl;
        }

    private:
        std::fstream file;
        std::string defaultFileName = "file.json";
};