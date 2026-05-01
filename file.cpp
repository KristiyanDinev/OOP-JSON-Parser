#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include "file.hpp"

// Creates (if does not exists) and opens a file stream, which will be used while editing the JSON
bool FileManager::openFile(std::string& filename) {
    file.clear();
    createFile(filename);
    file.open(filename, std::ios::in | std::ios::out);
    if (!file.is_open()) {
        file.clear();
        createFile(defaultFileName);
        file.open(defaultFileName, std::ios::in | std::ios::out);
        std::cout << "Can't open/create the file: " << filename << std::endl;
        if (!file.is_open()) {
            file.clear();
            std::cout << "Can't open/create the file: " << defaultFileName << std::endl;
            return false;
        }
        filename = defaultFileName;
    }
    currentFileName = filename;
    std::cout << "The file is open/create: " << filename << std::endl;
    return true;
}

// Create that file by opening it with the append IO flag
void FileManager::createFile(const std::string& name) const {
    std::fstream created(name, std::ios::app);
}

// Closes the current file and clear the file flags
void FileManager::closeFile() {
    if (file.is_open()) {
        file.close();
        std::cout << "The file is closed." << std::endl;
    }
    file.clear();
}

// Close and open the file as a new file (no context from original file) and save the given
// data inside the file
void FileManager::saveData(const std::string& data) {
    if (file.is_open()) {
        file.close();
        file.clear();
        file.open(currentFileName, std::ios::in | std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            std::cout << "The file is not open. Data cannot be written." << std::endl;
            return;
        }
        file << data << std::endl;
        file.flush();
        std::cout << "The data has been written to the file." << std::endl;
    } else {
        std::cout << "The file is not open. Data cannot be written." << std::endl;
    }
}

// Get the entire file content
std::string FileManager::readData() {
    if (!file.is_open()) {
        return "";
    }
    file.clear();
    file.seekg(0, std::ios::end);
    std::size_t size = file.tellg();
    std::string data;
    data.resize(size);

    file.seekg(0, std::ios::beg);
    file.read(data.data(), size);
    return data;
}

// Saves data to a new file and keep the new file open
void FileManager::saveAs(std::string& newFilename, const std::string& data) {
    if (file.is_open()) {
        file.close();
    }
    if (!openFile(newFilename)) {
        return;
    }
    saveData(data);
}
