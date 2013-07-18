cmake_minimum_required(VERSION 2.8)
project(pcore_badapple)

###
# Common Cmake switches
#
SET (PCORE_OPTIMIZE OFF)

# Setup pCore include directory.
INCLUDE_DIRECTORIES (${CMAKE_CURRENT_LIST_DIR}/include)

# Compiler flags.
IF (PCORE_OPTIMIZE)
  SET (PCORE_BASE_FLAGS "-g -O2 -Wall")
  SET (PCORE_CC_FLAGS "-g -O2 -Wall")
ELSE()
  SET (PCORE_BASE_FLAGS "-g -Wall")
  SET (PCORE_CC_FLAGS "-g -Wall")
ENDIF()
SET (CMAKE_C_FLAGS ${PCORE_BASE_FLAGS})
SET (CMAKE_CXX_FLAGS ${PCORE_CC_FLAGS})
SET (CMAKE_ASM_FLAGS ${PCORE_BASE_FLAGS})

###
# gendata executable.
#
FILE (GLOB_RECURSE GENDATA_SRCS src/*.c)

ADD_EXECUTABLE(
  gendata
  ${GENDATA_SRCS}
)

SET (FFMPEG_LIBRARIES
  avcodec
  avformat
  avutil
  swscale
)
TARGET_LINK_LIBRARIES (gendata ${FFMPEG_LIBRARIES})
