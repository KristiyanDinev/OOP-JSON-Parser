#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "json.h"
#include "file.h"
#include "menu.h"

int main() {
    FileManager* fileManager = new FileManager();
    JsonParser* jsonParser = new JsonParser();
    Menu menu = Menu(*fileManager, *jsonParser);
    while (true) {
        menu.executeCommand(menu.getCommand());
    }
    delete fileManager;
    delete jsonParser;
    return 0;
}