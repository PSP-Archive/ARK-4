--[[
	Updater Of App.
	Designed by DevDavisNunez to ONElua projects.. :D
	TODO:
	Maybe, extract in APP, and only installdir in this..
]]

buttons.homepopup(0)

color.loadpalette()

args = os.arg()
if args:len() == 0 then
	os.message("Error args lost!")
	os.exit()
end

args /= "&"
if #args != 3 then
	os.message("Error args lost!")
	os.exit()
end

function onAppInstall(step, size_argv, written, file, totalsize, totalwritten)

    if step == 1 then -- Only msg of state
		draw.fillrect(0,0,960,30, color.green:a(100))
		screen.print(10,10,"Search in vpk, Unsafe or Dangerous files!")
		screen.flip()
	elseif step == 2 then -- Alerta Vpk requiere confirmacion!
		return 10 -- Ok code
	elseif step == 3 then -- Unpack :P
		draw.fillrect(0,0,960,30, color.green:a(100))
		screen.print(10,10,"Unpack vpk...")
		screen.print(925,10,"Percent Total: "..math.floor((totalwritten*100)/totalsize).." %",1.0,color.white, color.black, __ARIGHT)
		screen.print(10,35,"File: "..tostring(file))
		screen.print(10,55,"Percent: "..math.floor((written*100)/size_argv).." %")
		draw.fillrect(0,544-30,(totalwritten*960)/totalsize,30, color.new(0,255,0))
		screen.flip()
	elseif step == 4 then -- Promote o install :P
		draw.fillrect(0,0,960,30, color.green:a(100))
		screen.print(10,10,"Installing...")
		screen.flip()
	end
end

game.install(args[3])
files.delete(args[3])

buttons.homepopup(1)

game.launch(args[2])