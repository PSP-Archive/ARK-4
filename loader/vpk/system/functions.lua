--------------------Check your Games (ONLY PSP GAMES)-----------------------------------------------------------------------
list = {data = {}, len = 0, icons = {}, picons = {} }
filenames = { fn = {}, len = 0 }

function credits()
	local pos = 1
	local y = 85
	local reverse = false
	local ft = font.load("resources/fonts/Neuton-Bold.ttf")
	font.setdefault(ft)
	while true do
		draw.gradrect(1, 1, 959, 543, color.blue, color.green, __DIAGONAL)
		buttons.read()
		screen.print(472, 20, "Credits", 1, color.red)	
		screen.print(472, 35, "----------", 1, color.red)
		screen.print(460, y, "Acid_Snake", 1, color.black, color.orange)
		screen.print(460, y+20, "Yoti", 1, color.black, color.orange)
		screen.print(460, y+40, "Krazynez", 1, color.black, color.orange)
		screen.print(430, y+60, "#Blame Cypress", 1, color.black, color.orange)
		if buttons.released.r then
			font.setdefault()
			break
		end
		if y > 409 and reverse == true then
			y -= 1 
		elseif y > 409 and reverse == false then
			reverse = true
		elseif y < 86 and reverse == true then
			reverse = false
		elseif reverse == true and y > 85 then
			y -= 1
		elseif reverse == false then
			y += 1
		end
		screen.flip()
	end
end

----------------------------Update DataBase---------------------------------------------------------------------------------
function update_db(flag)
	os.delay(1000)
	os.updatedb()
	if flag then
		os.message("Your PSVita will restart...\nRemember to activate Henkaku Again",0)
	else
		os.message("Your PSVita will restart...\nand your database will be rebuilt",0)
	end
	buttons.homepopup(1)
	os.delay(2500)
	power.restart()
end

------------------------Check your Free Space-------------------------------------------------------------------------------
function check_freespace()
	local info = os.devinfo("ux0:")
	if info then
		sizeUxo = info.free
		
		if (info.free > 40 * 1024* 1024) then return true
		else return false end
	end
end
