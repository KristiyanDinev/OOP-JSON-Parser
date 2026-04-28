#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "json.h"
#include "file.h"
#include "menu.h"

int main() {
    FileManager* fileManager = new FileManager();
    JsonManager* jsonManager = new JsonManager("");
    Menu menu = Menu(*fileManager, *jsonManager);
    while (true) {
        menu.executeCommand(menu.getCommand());
    }
    delete fileManager;
    delete jsonManager;
    return 0;
}