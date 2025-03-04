cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(VMLBuild)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(VMLTemplate-CS)

include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -marm -mtune=cortex-a9 -mfpu=neon -Wno-incompatible-pointer-types -Wno-stringop-overflow -Wno-deprecated-declarations")
# HACK: don't know why but SceLibcMonoBridge causes problems with some C symbols, must use libsupc++ manually to link some c++ symbols
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-multiple-definition")

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

#add_subdirectory(lib)
add_subdirectory(src)