#!/bin/bash

code="$PWD"
opts=-g
cd . > /dev/null
g++ $opts $code/4ed.exe -o 4ed.exe
cd $code > /dev/null
