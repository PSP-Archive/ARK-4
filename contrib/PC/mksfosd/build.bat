@echo off
title Savedata Signer Script by Yoti
echo Savedata Signer Script by Yoti
if not exist endecrypter.exe (
	echo Please build endecrypter.exe first!
	goto end
)
if not exist mksfosd.exe (
	echo Please build mksfosd.exe first!
	goto end
)
if not exist PARAM.SFO (
	mksfosd.exe 0 "ARK-4" "ARK_01234" "SAVEDATA.BIN" 1 "ARK-4" "ARK-4"
)
if not exist SAVEDATA.BIN (
	endecrypter.exe ARK01234.BIN ARK01234.BIN PARAM.SFO > SAVEDATA.BIN
)
:end
pause