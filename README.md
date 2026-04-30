# JSON Parser

Console application for reading, validating, modifying, and saving JSON files without external JSON libraries.

> What is JSON? https://www.w3schools.com/js/js_json.asp

## Purpose

Provide a compact command-driven workflow for inspecting and editing JSON documents.

## Build and Run

This project is a console application. From the workspace root, compile the program with:

```text
g++ -std=c++17 -g main.cpp -o json_parser.exe
```

Then run it with:

```text
.\json_parser.exe
```

If you prefer to build the test programs, compile each file in `tests/` separately using the same compiler flag style.

## C++ Version

The code is compatible with C++11 and later. C++17 is recommended for building and testing the project.

## Terms

- **Key**: A member name used to locate values, for example `Documents`.
- **Path**: Slash-delimited keys or indices, for example `Documents/Health/name` or `Shopping/3/name`.

Note: `set` and `create` accept primitive values only.

## File Commands

- `open [/path/to/file]` - Opens a JSON file for processing. If it does not exist, a new file named `file.json` is created in that location.
- `close` - Closes the file without saving.
- `save` - Saves changes to the currently opened file.
- `saveas` - Saves changes to a new file path.
- `help` - Prints the help menu.
- `exit` - Exits the program without saving.

## JSON Commands

- `validate` - Reports whether the loaded JSON is valid.
- `print` - Prints the JSON with indentation.
- `search <key>` - Prints an array with all values that match the key.
- `set <path> <string>` - Updates an existing path with a new value.
- `create <path> <string>` - Creates a new path with a value.
- `delete <path>` - Deletes a path and its value.
- `move <from path> <to path>` - Moves a value from one path to another.

## Examples (using example.json)

Assume the working directory contains `example.json` with the provided sample content.

### open

```text
open example.json
```

Expected result:
```text
The file is open/create: example.json
```

### help

```text
help
```

Expected result: the full command list is printed.

### validate

```text
validate
```

Expected result:
```text
Valid JSON.
```

### print

```text
print
```

Expected result: formatted output of the JSON document.

### search

```text
search name
```

Expected result (values with the key `name`):
```text
Search results for key 'name': [
	"Health Document",
	"House Document",
	"Batteries"
]
```

### set

```text
set Documents/Health/name "Updated Health Document"
```

Expected result:
```text
Set Documents/Health/name with the value "Updated Health Document"
```

### create

```text
create Documents/Health/owner "Alice"
```

Expected result:
```text
Created Documents/Health/owner with value "Alice"
```

### delete

```text
delete Shopping/0
```

Expected result:
```text
Deleted Shopping/0
```

### move

```text
move Hello Greeting
```

Expected result:
```text
Moved from Hello to Greeting
```

### save

```text
save
```

Expected result:
```text
The data has been written to the file.
```

### saveas

```text
saveas "example-copy.json"
```

Expected result:
```text
The data has been written to the file.
```

### close

```text
close
```

Expected result:
```text
The file is closed.
```

### exit

```text
exit
```

Expected result: the program terminates without saving.

## Unit Tests

Three standalone test files are included under `tests/` (no external libraries or JSON parsers used):

- `tests/json_tests.cpp`: JsonParser parsing/validation, search, create/set/delete/move, and error handling.
- `tests/file_tests.cpp`: FileManager open/close/save/saveas/read, including fallback behavior when a path cannot be opened.
- `tests/menu_tests.cpp`: Menu command parsing and command execution for open/save/close.

### Running the tests (g++)

From the workspace root:

```text
g++ -std=c++17 -g tests/json_tests.cpp -o tests/json_tests.exe
tests\\json_tests.exe

g++ -std=c++17 -g tests/file_tests.cpp -o tests/file_tests.exe
tests\\file_tests.exe

g++ -std=c++17 -g tests/menu_tests.cpp -o tests/menu_tests.exe
tests\\menu_tests.exe
```

Note: The tests create temporary JSON files in the working directory and clean them up.
