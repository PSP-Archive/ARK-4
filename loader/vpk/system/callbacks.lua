-- CallBack CopyFiles
function onCopyFiles(size,written,file)
	if status then
		if back then back:blit(0,0) end
		draw.fillrect(0,0,__DISPLAYW,30, color.new(255,255,255,100) )

		if (count+1)<=number_clons then
			screen.print(10,10,"Cloning Bubble: "..count+1)
			screen.print(10,35,"File: "..tostring(mgsid))
			screen.print(10,55,"Percent: "..math.floor((written*100)/size).." %")
		end

		screen.flip()
	end
	return 1
end

-- CallBack Extraction
function onExtractFiles(size,written,file,totalsize,totalwritten)
	if back then back:blit(0,0) end
	draw.fillrect(0,0,__DISPLAYW,30, color.new(255,255,255,100) )

	screen.print(10,10,"Extracting and Installing...")
	screen.print(10,30,"File: "..tostring(file))
	screen.print(10,50,"Percent: "..math.floor((written*100)/size).." %")
	screen.print(10,70,"Percent Total: "..math.floor((totalwritten*100)/totalsize).." %")
	screen.print(10,90,"Size Total: "..tostring(totalsize).." ".."Written: "..tostring(totalwritten))

	screen.flip()
	return 1
end
