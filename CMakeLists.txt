cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(VML/VMLBuild)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(VMLTemplate-CS)

include("${VITASDK}/share/vita.cmake" REQUIRED)

set(USE_LINKER_SCRIPT OFF CACHE STRING "Use custom linker script (for big ports)")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-incompatible-pointer-types -Wno-stringop-overflow -Wno-deprecated-declarations")
# HACK: don't know why but SceLibcMonoBridge causes problems with some C symbols, must use libsupc++ manually to link some c++ symbols
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-multiple-definition")

if(USE_LINKER_SCRIPT)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${CMAKE_SOURCE_DIR}/extras/linker/vmlport.ld")
endif()

add_custom_target(ExternalVMLFNA)

# FNA imports
compile_mono_external_import(
    ASSEMBLY SDL2.dll
    TARGET ExternalVMLFNA
)

compile_mono_external_import(
    ASSEMBLY SDL2_image.dll
    TARGET ExternalVMLFNA
)

compile_mono_external_import(
    ASSEMBLY SDL2_mixer.dll
    TARGET ExternalVMLFNA
)

compile_mono_external_import(
    ASSEMBLY FAudio-CS.dll
    TARGET ExternalVMLFNA
)

compile_mono_external_import(
    ASSEMBLY Theorafile.dll
    TARGET ExternalVMLFNA
)

compile_mono_external_import(
    ASSEMBLY FNA.dll
    TARGET ExternalVMLFNA
)

set(VML_CORE_C_LIBRARY SceLibcMonoBridge_stub)

#add_subdirectory(lib)
add_subdirectory(src)