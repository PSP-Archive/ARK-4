cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(ef2uma)
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

add_executable(ef2uma
  ef2uma.c
)

target_link_libraries(ef2uma
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

set_target_properties(ef2uma
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(ef2uma.suprx ef2uma
  CONFIG ${CMAKE_SOURCE_DIR}/ef2uma.yml
)
