#!/usr/bin/env bash

# ArkFast updater script by Yoti for ARK-4 project
# 2022-09-06: initial release
# 2022-09-06: fix kexploit -1
# 2022-09-23: cleanup files
# 2023-02-10: update version
# 2023-05-14: fix for v4.20
# 2023-06-08: zip/unzip checks
# 2023-06-08: custom build ver

#sudo apt update
#sudo apt install zip unzip -y

if [ "$1" == "" ]; then
    ark_zip_link="https://github.com/PSP-Archive/ARK-4/releases/latest/download/ARK4.zip"
else
    ark_zip_link="${1//tag/download}""/ARK4.zip"
fi

if ! command -v zip &> /dev/null
then
    echo "Please install zip first"
    exit
fi

if ! command -v unzip &> /dev/null
then
    echo "Please install unzip first"
    exit
fi

if [ -f "ArkFast.vpk" ]; then
    rm -f ArkFast.vpk
fi
wget -q --show-progress "https://github.com/ONElua/ArkFast/releases/latest/download/ArkFast.vpk"
if [ -f "ArkFast.vpk" ]; then
    unzip ArkFast.vpk -d ArkFast_tmp
    rm -f ArkFast.vpk
else
    echo "Can't found ArkFast.vpk"
    exit
fi

if [ -f "ARK4.zip" ]; then
    rm -f ARK4.zip
fi
wget -q --show-progress $ark_zip_link
if [ -f "ARK4.zip" ]; then
    unzip ARK4.zip -d ARK4_tmp
    rm -f ARK4.zip
else
    echo "Can't found ARK4.zip"
    exit
fi

if [ -f "ArkFast_new.vpk" ]; then
    rm -f ArkFast_new.vpk
fi
cd ARK4_tmp/
mv -f Vita/Standalone/K.BIN ARK_01234/K.BIN
zip -r ARK_01234.zip ARK_01234
mv -f ARK_01234.zip ../ArkFast_tmp/resources/ARK_01234.zip
cd ..
rm -rf ARK4_tmp/
cd ArkFast_tmp/
sed -i 's/ARK-2/ARK-4/g' script.lua
zip -r ../ArkFast_new.vpk .
cd ..
rm -rf ArkFast_tmp/

echo "Done!"
