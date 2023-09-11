cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(FasterARK)

include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "FasterARK")
set(VITA_TITLEID  "FASTERARK")

set(VITA_VERSION  "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -fno-lto")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -s CATEGORY=gdb")
set(VITA_MAKE_FSELF_FLAGS "${VITA_MAKE_FSELF_FLAGS}")

include_directories(
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME}  
  src/ui.c
  src/install.c
  src/io.c
  src/promote.c
  src/pbp.c
  src/sha256
  src/main.c
)

target_link_libraries(${PROJECT_NAME}
	vita2d
	z
	c
	m
	png
	jpeg
	SceGxm_stub
	SceSysmodule_stub 
	ScePgf_stub
	ScePvf_stub
	SceCtrl_stub
	SceCommonDialog_stub 
	
	SceLibKernel_stub
	SceIofilemgr_stub
	SceAppMgr_stub
	SceDisplay_stub
	SceVshBridge_stub
	ScePromoterUtil_stub
	taihen_stub
)

vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE)

vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}

  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
  
  FILE res/rif/game.rif rif/game.rif
  FILE res/psp/EBOOT.PBP psp/EBOOT.PBP
  FILE res/psp/PBOOT.PBP psp/PBOOT.PBP
  FILE res/save/ARK_01234/ARK.BIN save/ARK_01234/ARK.BIN
  FILE res/save/ARK_01234/ARK4.BIN save/ARK_01234/ARK4.BIN
  FILE res/save/ARK_01234/ARKX.BIN save/ARK_01234/ARKX.BIN
  FILE res/save/ARK_01234/FLASH0.ARK save/ARK_01234/FLASH0.ARK
  FILE res/save/ARK_01234/H.BIN save/ARK_01234/H.BIN
  FILE res/save/ARK_01234/ICON0.PNG save/ARK_01234/ICON0.PNG
  FILE res/save/ARK_01234/IDSREG.PRX save/ARK_01234/IDSREG.PRX
  FILE res/save/ARK_01234/K.BIN save/ARK_01234/K.BIN
  FILE res/save/ARK_01234/LANG.ARK save/ARK_01234/LANG.ARK
  FILE res/save/ARK_01234/MEDIASYN.PRX save/ARK_01234/MEDIASYN.PRX
  FILE res/save/ARK_01234/PARAM.SFO save/ARK_01234/PARAM.SFO
  FILE res/save/ARK_01234/POPS.PRX save/ARK_01234/POPS.PRX
  FILE res/save/ARK_01234/POPSMAN.PRX save/ARK_01234/POPSMAN.PRX
  FILE res/save/ARK_01234/PS1SPU.PRX save/ARK_01234/PS1SPU.PRX
  FILE res/save/ARK_01234/RECOVERY.PBP save/ARK_01234/RECOVERY.PBP
  FILE res/save/ARK_01234/RECOVERY.PRX save/ARK_01234/RECOVERY.PRX
  FILE res/save/ARK_01234/SAVEDATA.BIN save/ARK_01234/SAVEDATA.BIN
  FILE res/save/ARK_01234/THEME.ARK save/ARK_01234/THEME.ARK
  FILE res/save/ARK_01234/UPDATER.TXT save/ARK_01234/UPDATER.TXT
  FILE res/save/ARK_01234/USBDEV.PRX save/ARK_01234/USBDEV.PRX
  FILE res/save/ARK_01234/VBOOT.PBP save/ARK_01234/VBOOT.PBP
  FILE res/save/ARK_01234/VSHMENU.PRX save/ARK_01234/VSHMENU.PRX
  FILE res/save/ARK_01234/XBOOT.PBP save/ARK_01234/XBOOT.PBP
  FILE res/save/ARK_01234/XMBCTRL.PRX save/ARK_01234/XMBCTRL.PRX
)
