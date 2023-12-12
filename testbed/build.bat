REM Build Script for testbed
@ECHO OFF
SetLocal EnableDelayedExpansion

REM Get a list of all the .c files.
SET cFileNames=
FOR /R %%f in (*.c) do (
    SET cFileNames=!cFileNames! %%f
)

REM echo "Files: " %cFileNames%

SET assembly=testbed
SET compilerFlags=-g
REM -Wall -Werror
SET includeFlags= -Isrc -I../engine/src/
SET linkerFlags= -L../bin/ -lengine
SET defines= -D_DEBUG -DVIMPORT

ECHO "Building %assembly%%..."
clang %compilerFlags% %cFileNames% -o ../bin/%assembly%.exe %defines% %includeFlags% %linkerFlags%