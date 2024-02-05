cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(ps1cfw_enabler)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

include_directories(
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

if (NOT ${RELEASE})
  add_definitions(-DENABLE_LOGGING)
endif()

add_executable(ps1cfw_enabler
  ps1cfw_enabler.c
)

target_link_libraries(ps1cfw_enabler
  taihen_stub
  SceLibKernel_stub
  SceLibc_stub_weak
  SceCompat_stub
  SceKernelModulemgr_stub
  SceIofilemgr_stub
  SceRtc_stub
  SceCtrl_stub
  SceDisplay_stub
  SceShellSvc_stub
  SceProcessmgr_stub
)

set_target_properties(ps1cfw_enabler
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(ps1cfw_enabler.suprx ps1cfw_enabler
  CONFIG ${CMAKE_SOURCE_DIR}/ps1cfw_enabler.yml
)
