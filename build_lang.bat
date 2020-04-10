@echo off

SET LANG=%1

call custom\bin\build_one_time.bat custom\languages\%LANG%\lexer_gen.cpp
call one_time
del one_time.exe
del *.ilk
del *.pdb
del *.obj