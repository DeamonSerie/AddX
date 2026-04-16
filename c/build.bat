@echo off
rem Simple build script for AddX C components
rem Builds tokenizer, parser, ast, avm into a static library and links a test program

rem Check if cl.exe is available
where cl >nul 2>&1
if %errorlevel% neq 0 (
    echo cl.exe not found in PATH. Please install Visual Studio Build Tools or use gcc.
    exit /b 1
)

rem Create output directory
if not exist bin mkdir bin

rem Compile individual source files
cl /c /nologo /W4 tokenizer.c
cl /c /nologo /W4 parser.c
cl /c /nologo /W4 ast.c
cl /c /nologo /W4 avm.c
cl /c /nologo /W4 compiler.c
cl /c /nologo /W4 vm.c

rem Create static library
lib /nologo /OUT:bin\libaddx.lib tokenizer.obj parser.obj ast.obj avm.obj compiler.obj vm.obj

rem Compile a simple test program that uses the parser and tokenizer
cl /c /nologo /W4 test_parse_source.c
cl /nologo /Fe:bin\test_parse_source.exe test_parse_source.obj bin\libaddx.lib

rem Compile the simple_main AVm test
cl /c /nologo /W4 simple_main.c
cl /nologo /Fe:bin\simple_main.exe simple_main.obj bin\libaddx.lib

echo Build complete. Executables are in bin\ directory.
pause