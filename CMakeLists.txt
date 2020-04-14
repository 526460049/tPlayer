# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("tPlayer")

# 包含子项目。
#add_subdirectory ("tPlayer")
add_executable (${PROJECT_NAME} 
	"src/main.cpp"
	"src/ClientUtil.cpp" 
	"src/LivePage2.cpp")


	include(./.SET_LOCAL_PATH.cmake)

######################################
#  SDL2 lib setting
######################################
#make sure SDL2.dll is in your PATH.

find_path(SDL_INCLUDE_DIR "SDL.h")
find_library(SDL_LIBRARY "SDL2.lib")

######################################
#  ffmpeg lib setting
######################################
find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY "avcodec.lib")

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY "avformat.lib")

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY "avutil.lib")

find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
find_library(AVDEVICE_LIBRARY "avdevice.lib")

find_path(AVFILTER_INCLUDE_DIR libavfilter/avfilter.h)
find_library(AVFILTER_LIBRARY "avfilter.lib")

find_path(POSTPROC_INCLUDE_DIR libpostproc/postprocess.h)
find_library(POSTPROC_LIBRARY "postproc.lib")

find_path(SWRESAMPLE_INCLUDE_DIR libswresample/swresample.h)
find_library(SWRESAMPLE_LIBRARY "swresample.lib")

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)
find_library(SWSCALE_LIBRARY "swscale.lib")


######################################
#  FLTK lib setting
######################################
find_path(FLTK_INCLUDE_DIR "FL")

find_library(FLTKD_LIBRARY "fltkd.lib")
find_library(FLTKGLD_LIBRARY "fltkgld.lib")
find_library(FLTKFORMD_LIBRARY "fltkformsd.lib")
find_library(FLTKJPEGD_LIBRARY "fltkjpegd.lib")
find_library(FLTKPNGD_LIBRARY "fltkpngd.lib")
find_library(FLTKZLIBD_LIBRARY "fltkzlibd.lib")


# 配置链接库
target_link_libraries( ${PROJECT_NAME}  
	PRIVATE 
		${FLTKD_LIBRARY}
		${FLTKIMAGED_LIBRARY}
		${FLTKGLD_LIBRARY}
		${FLTKFORMD_LIBRARY}
		${FLTKJPEGD_LIBRARY}
		${FLTKPNGD_LIBRARY}
		${FLTKZLIBD_LIBRARY}
		${AVCODEC_LIBRARY} 
		${AVFORMAT_LIBRARY} 
		${AVUTIL_LIBRARY} 
		${AVDEVICE_LIBRARY}
		${AVFILTER_LIBRARY}
		${POSTPROC_LIBRARY}
		${SWRESAMPLE_LIBRARY}
		${SWSCALE_LIBRARY}
		${SDL_LIBRARY}
)
target_include_directories( ${PROJECT_NAME}  
	PRIVATE 
		${PROJECT_SOURCE_DIR}/include
		${FLTK_INCLUDE_DIR}
		${AVCODEC_INCLUDE_DIR} 
		${AVFORMAT_INCLUDE_DIR} 
		${AVUTIL_INCLUDE_DIR} 
		${AVDEVICE_INCLUDE_DIR}
		${AVFILTER_INCLUDE_DIR}
		${POSTPROC_INCLUDE_DIR}
		${SWRESAMPLE_INCLUDE_DIR}
		${SWSCALE_INCLUDE_DIR}
		${SDL_INCLUDE_DIR}
)
