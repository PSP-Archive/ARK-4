#!/usr/bin/env bash
#
# Author: Krazynez
#
# Date  : August 26, 2023
#

if [-f "FastARK-4.vpk" ]; then
	rm "FastARK-4.vpk"
fi

curl_check=$(command -v curl)
curl_ret=$?
if [ $curl_ret == 0 ]; then
	$curl_check -OJL "https://github.com/PSP-Archive/ARK-4/releases/latest/download/ARK4.zip"
else
	$(command -v wget) "https://github.com/PSP-Archive/ARK-4/releases/latest/download/ARK4.zip"
fi

unzip_check=$(command -v unzip)

unzip_ret=$?

if [ $unzip_ret != 0 ]; then
	printf "Please install unzip with your package manager\n"
	exit 1;
fi

mkdir ARK_01234

unzip -j ARK4.zip ARK_01234/* -d ARK_01234/
unzip -oj ARK4.zip PSVita/Standalone/K.BIN -d ARK_01234/

zip -r ARK_01234.zip ARK_01234/*

rm -rf ARK_01234/ ARK4.zip

mv ARK_01234.zip resources/

zip -r FastARK-4.vpk *


