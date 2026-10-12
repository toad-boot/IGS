@echo off
cd /d "%~dp0"
if not exist bin mkdir bin

g++ -O2 -DUNICODE -D_UNICODE -mwindows -static Part1_Graphs\Graphs.cpp -o bin\Graphs.exe
windres -c 65001 Part2_Menu\MenuWin.rc -O coff -o bin\MenuWin.res
g++ -O2 -DUNICODE -D_UNICODE -mwindows -static Part2_Menu\MenuWin.cpp bin\MenuWin.res -o bin\MenuWin.exe
del bin\MenuWin.res
