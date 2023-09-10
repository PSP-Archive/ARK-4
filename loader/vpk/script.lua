--------------------UNSAFE or SAFE-----------------------------------------------------------------------------
if os.access() == 0 then
	if os.master() == 1 then os.restart()
	else
		os.message("UNSAFE MODE is required for this HB!",0)
		os.exit()
	end
end
if files.exists("ux0:pspemu/temp/") then files.delete("ux0:pspemu/temp/") end

__NAMEVPK = "ARK-4 Installer"
dofile("updater.lua")

----------------------Vars and resources------------------------------------------------------------------------------------
color.loadpalette()
back = image.load("back.png")
buttonskey = image.load("buttons.png",20,20)
buttonskey2 = image.load("buttons2.png",30,20)
status,sizeUxo,clon,pos,dels=false,0,0,1,0
actfile = files.exists("tm0:npdrm/act.dat")
mgsid=""

--constants
PATHTOGAME = "ux0:pspemu/PSP/GAME/"
PATHTOCLON = "ur0:appmeta/"
PATHTOLICENSE = "ux0:pspemu/PSP/LICENSE/"

dofile("system/ark.lua")
dofile("system/functions.lua")
dofile("system/callbacks.lua")

------------------------Menu Principal--------------------------------------------------------------------------------------
check_freespace()
files.mkdir(PATHTOGAME)
buttons.interval(10,10)

while true do
	buttons.read()
	if back then back:blit(0,0) end

	--Show size free
	screen.print(950,10,"ux0: "..files.sizeformat(sizeUxo).." Free",1,color.white,color.blue,__ARIGHT)

	if buttonskey then
		buttonskey:blitsprite(50,308,0)
	end
		
	screen.print(80,310,"Install ARK-4",1,color.white,color.blue)

	screen.print(80, 340, "Credits",1,color.white,color.blue)
	buttonskey2:blitsprite(50,338,0)

	if buttons.cross then 
		if check_freespace() then 
			install_ark_from0()
		else
			os.message("Not Enough Memory (minimum 40 MB)")
		end
	end

	---------Controls-------------------------------------------------------------------------------------------------------
	if buttons.held.r then
		credits()
	end
	if buttons.triangle and list.data[pos].clon == "©" then
		list.data[pos].del = not list.data[pos].del
		if list.data[pos].del then dels+=1 else dels-=1 end
	end

	if buttons.start then
		if check_freespace() then
			if files.exists(PATHTONPUZ.."/EBOOT.PBP") then
				os.message("The MINIS Sasuke vs Commander is already installed",0)
			else
				install_ark_from0()
			end
		else
			os.message("Not Enough Memory (minimum 40 MB)")
		end
	end
		---------Controls---------------------------------------------------------------------------------------------------

	screen.flip()
end
