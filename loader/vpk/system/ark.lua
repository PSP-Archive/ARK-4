------Install PSP MINI NPUZ00146 (include ARK_01234 & PBOOT)
function install_ark_from0()
	buttons.homepopup(0)

	local pathsave = "ux0:/pspemu/PSP/SAVEDATA/"
	if not files.exists(pathsave) then files.mkdir(pathsave) end
	files.extract(files.cdir().."/resources/ARK_01234.zip",pathsave)

	if files.exists("resources/NPUZ01234/") and files.exists("resources/NPUZ01234/") then
		files.copy("resources/NPUZ01234/", "ux0:pspemu/PSP/GAME/")
		files.copy("resources/LICENSE/*.rif", "ux0:pspemu/PSP/LICENSE/")
	else
		os.message("Looks like NPUZ01234 is missing. :-(")
		buttons.homepopup(1)
		return
	end
	os.updatedb()
	--os.rebuilddb()
	os.delay(2500)
	buttons.homepopup(1)

	os.message('PS Vita will restart and finish installing ARK-4')
	power.restart()
end
