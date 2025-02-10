cmake_minimum_required(VERSION 3.18)

set(IRRKLANG_DIR "${CMAKE_SOURCE_DIR}/libs/irrklang")

set(IRRKLANG_LIB "${IRRKLANG_DIR}/irrKlang.lib")
set(IRRKLANG_DLL "${IRRKLANG_DIR}/irrKlang.dll")
set(IKPMP3_DLL "${IRRKLANG_DIR}/ikpMP3.dll")

message(STATUS "Using irrKlang from: ${IRRKLANG_DIR}")
message(STATUS "IrrKlang Library: ${IRRKLANG_LIB}")
message(STATUS "IrrKlang DLL: ${IRRKLANG_DLL}")
message(STATUS "IkpMP3 DLL: ${IRRKLANG_DLL}")
