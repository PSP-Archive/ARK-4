#!/usr/bin/env bash

if [ "$1" != "" ]; then
    find "$1" -iname "*.c" -or -iname "*.h" -or -name "*.cpp" -exec sed -i "s/\t/    /" {} \;
else
    find . -iname "*.c" -or -iname "*.h" -or -name "*.cpp" -exec sed -i "s/\t/    /" {} \;
fi
