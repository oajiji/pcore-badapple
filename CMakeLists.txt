cmake_minimum_required(VERSION 2.8)
project(pcore_badapple)

###
# Common Cmake switches
#
SET (PCORE_OPTIMIZE ON)

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
SET (GENDATA_SRCS src/lz4.c src/gendata.c)
SET (PLAYDAT_SRCS src/lz4.c src/playdat.cc)
SET (MKIMG_SRCS src/lz4.c src/mkimg.cc)

ADD_EXECUTABLE(
  gendata
  ${GENDATA_SRCS}
)
ADD_EXECUTABLE(
  playdat
  ${PLAYDAT_SRCS}
)
ADD_EXECUTABLE(
  mkimg
  ${MKIMG_SRCS}
)

SET (FFMPEG_LIBRARIES
  avcodec
  avformat
  avutil
  swscale
  opencv_calib3d 
  opencv_contrib 
  opencv_core 
  opencv_features2d 
  opencv_flann 
  opencv_highgui 
  opencv_imgproc 
  opencv_legacy 
  opencv_ml 
  opencv_objdetect 
  opencv_photo 
  opencv_stitching 
  opencv_ts 
  opencv_video 
  opencv_videostab
)
TARGET_LINK_LIBRARIES (gendata ${FFMPEG_LIBRARIES})
TARGET_LINK_LIBRARIES (playdat ${FFMPEG_LIBRARIES})
TARGET_LINK_LIBRARIES (mkimg ${FFMPEG_LIBRARIES})
