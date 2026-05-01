
#pragma once

#include <string>

#include "file.hpp"
#include "json.hpp"

#pragma once

#include <string>

class FileManager;
class JsonParser;

enum CommandEnum {
    OPEN,
    CLOSE,
    SAVE,
    SAVEAS,
    HELP,
    EXIT,
    VALIDATE,
    PRINT,
    SEARCH,
    SET,
    CREATE,
    DELETE,
    MOVE
};

struct Command
{
    std::string arg1;
    std::string arg2;
    CommandEnum commandEnum;
};

class Menu {
    public:
        Menu(FileManager& fileManager, JsonParser& jsonParser);

        void printHelp();
        std::string getInput();
        Command getCommand(const std::string& input);
        void executeCommand(Command cmd);

    private:
        FileManager& fileManager;
        JsonParser& jsonParser;

        std::string toLowerCase(char* input, int size);
        void getCommandEnumFromInput(const std::string& input, Command& cmd, unsigned int commandStopInd);
        void parseArguments(const std::string& input, Command& cmd, unsigned int commandStopInd);
};