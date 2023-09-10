function onAppInstallCB(step, size_argv, written, file, totalsize, totalwritten)
	return 10 -- Ok code
end

function onNetGetFileCB(size,written,speed)
	if back then back:blit(0,0) end
	screen.print(10,10,"Downloading Update...")
	screen.print(10,30,"Size: "..tostring(size).." Written: "..tostring(written).." Speed: "..tostring(speed).."Kb/s")
	screen.print(10,50,"Porcent: "..math.floor((written*100)/size).."%")
	draw.fillrect(0,520,((written*960)/size),24,color.new(0,255,0))
	screen.flip()
	buttons.read()
	if buttons.circle then	return 0 end --Cancel or Abort
	return 1;
end

if files.exists("ux0:/app/ONEUPDATE") then
	game.delete("ONEUPDATE") -- Exists delete update app
end

UPDATE_PORT = channel.new("UPDATE_PORT")

local scr_flip = screen.flip
function screen.flip()
	scr_flip()
	if UPDATE_PORT:available() > 0 then
		local version = UPDATE_PORT:pop()
		local major = (version >> 0x18) & 0xFF;
		local minor = (version >> 0x10) & 0xFF;
		if os.message(__NAMEVPK.." "..string.format("%X.%02X",major, minor).." is now available.\n".."Do you want to update the application?", 1) == 1 then
			buttons.homepopup(0)
			
			local url = string.format("https://github.com/ONElua/ArkFast/releases/download/%s/%s", string.format("%X.%02X",major, minor),__NAMEVPK..".vpk")
			local path = "ux0:data/"..__NAMEVPK..".vpk"
			onAppInstall = onAppInstallCB
			onNetGetFile = onNetGetFileCB
			local res = http.getfile(url, path)
			if res then -- Success!
				files.mkdir("ux0:/data/1luapkg")
				files.copy("eboot.bin","ux0:/data/1luapkg")
				files.copy("updater/script.lua","ux0:/data/1luapkg/")
				files.copy("updater/param.sfo","ux0:/data/1luapkg/sce_sys/")
				game.installdir("ux0:/data/1luapkg")
				files.delete("ux0:/data/1luapkg")
				game.launch(string.format("ONEUPDATE&%s&%s",os.titleid(),path)) -- Goto installer extern!
			end
			
			buttons.homepopup(1)
		end
	end
end

THID = thread.new("thread_net.lua")