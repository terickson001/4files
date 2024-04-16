@echo off
set dirs=fonts\ themes\
set files=4ed.exe 4ed_app.dll bindings.4coder config.4coder custom_4coder.dll custom_4coder.pdb mac-bindings.4coder

for %%d in (%dirs%) do (robocopy /s .\%%d release\%%d)
for %%f in (%files%) do (robocopy . release %%f)
