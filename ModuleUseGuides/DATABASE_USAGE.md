# AddX DatabaseExt Module

Custom database handler system for AddX.

## Overview

DatabaseExt allows programmers to define their own logic for:
- Searching database records
- Grabbing/fetching data files
- Editing files or records
- Removing files or records
- Executing custom commands

## Handler

The Handler defines the interface for database operations:

```
Handler {
    search: (query: string) -> list<row>
    grab: (path: string) -> data
    edit: (path: string, content: data) -> bool
    remove: (path: string) -> bool
    execute: (command: string) -> result
}
```

## Creating a Custom Handler

### Simple Example: File System Handler

```
def fileSearch(query: string) -> list<row>
    results = []
    # Search files in current directory
    for file in listFiles(".") do
        if contains(file.name, query) then
            results.append({path: file.path, name: file.name})
    return results

def fileGrab(path: string) -> data
    return readFile(path)

def fileEdit(path: string, content: data) -> bool
    writeFile(path, content)
    return true

def fileRemove(path: string) -> bool
    deleteFile(path)
    return true

def fileExecute(command: string) -> result
    return runCommand(command)

handler = createHandler(
    searchFn = fileSearch,
    grabFn = fileGrab,
    editFn = fileEdit,
    removeFn = fileRemove,
    executeFn = fileExecute
)
```

### JSON Database Handler

```
def jsonSearch(query: string) -> list<row>
    data = loadJson("database.json")
    results = []
    for item in data do
        if contains(toString(item), query) then
            results.append(item)
    return results

def jsonGrab(path: string) -> data
    return parseJson(readFile(path))

def jsonEdit(path: string, content: data) -> bool
    writeFile(path, toJson(content))
    return true

def jsonRemove(path: string) -> bool
    deleteFile(path)
    return true

def jsonExecute(command: string) -> result
    return executeQuery(command)

handler = createHandler(jsonSearch, jsonGrab, jsonEdit, jsonRemove, jsonExecute)
```

### CSV Database Handler

```
def csvSearch(query: string) -> list<row>
    lines = readFile("data.csv").split("\n")
    results = []
    header = lines[0].split(",")
    for i in range(1, len(lines)) do
        if contains(lines[i], query) then
            row = {}
            values = lines[i].split(",")
            for j in range(len(header)) do
                row[header[j]] = values[j]
            results.append(row)
    return results

def csvGrab(path: string) -> data
    return parseCsv(readFile(path))

def csvEdit(path: string, content: data) -> bool
    writeFile(path, toCsv(content))
    return true

def csvRemove(path: string) -> bool
    deleteFile(path)
    return true

def csvExecute(command: string) -> result
    # Execute SQL-like command on CSV
    return executeCsvQuery(command)
```

## Registering a Database

```
# Create database language
myDB = createDatabase("MyFiles", handler, [".txt", ".json", ".csv"])

# Register it
registerCustom(myDB)
```

## Using a Database

After registration, use the Operations functions:

```
# Search
results = search("MyFiles", "example")

# Grab data
data = grab("MyFiles", "data/file.txt")

# Edit
success = edit("MyFiles", "data/file.txt", "new content")

# Remove
success = remove("MyFiles", "data/file.txt")

# Execute command
result = execute("MyFiles", "SELECT * FROM users")
```

## Listing Databases

```
# List all registered databases
names = Registry.list()
print(names)  # ["MyFiles", "JSONDB", "CSVDB"]

# Check if database exists
db = Registry.get("MyFiles")
if db != nothing then
    print("Found: " + db.name)
```

## Unregistering

```
unregisterCustom("MyFiles")
```

## Example: Full Custom Database

```
def main()
    # Create handler with custom logic
    handler = createHandler(
        searchFn = def(query: string) -> list<row>
            results = []
            # Custom search implementation
            return results
        end,
        grabFn = def(path: string) -> data
            # Custom grab implementation  
            return loadData(path)
        end,
        editFn = def(path: string, content: data) -> bool
            # Custom edit implementation
            saveData(path, content)
            return true
        end,
        removeFn = def(path: string) -> bool
            # Custom remove implementation
            deleteData(path)
            return true
        end,
        executeFn = def(command: string) -> result
            # Custom execute implementation
            return runCustomCommand(command)
        end
    )
    
    # Create and register database
    db = createDatabase("CustomDB", handler, [".dat"])
    registerCustom(db)
    
    # Use it
    results = search("CustomDB", "test")
    grab("CustomDB", "data.dat")
    edit("CustomDB", "data.dat", {key: "value"})
    
    # Unregister when done
    unregisterCustom("CustomDB")
    
    return 0
```