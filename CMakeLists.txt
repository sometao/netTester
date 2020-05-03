# CMakeList.txt : CMake project for helloLibevent, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.11)

project ("NetTester" VERSION 0.0.1)



include(./cmake/checkBuildType.cmake)

#-------------------------------------
#  spdlog settings
#-------------------------------------
include(./cmake/utils.cmake)


include(./.SET_LOCAL_PATH.cmake)


# --------------------------------------------------------------------------
# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-D__STDC_CONSTANT_MACROS")



if(MSVC)
    SET(CMAKE_CXX_FLAGS "/EHsc")  # deal with MSVC Warning C4530
endif(MSVC)


#-------------------------------------
#  ws2_32 for windows
#-------------------------------------
if(WIN32) # Check if we are on Windows
  find_library(WS2_LIBRARY ws2_32)
endif()




#-------------------------------------
#  PRE BUILD DEFINE
#-------------------------------------
if(WIN32)

elseIf(UNIX)
	add_definitions(-Wall)
endif()



#-------------------------------------
#  Threads
#-------------------------------------
find_package(Threads REQUIRED) 



#-------------------------------------
#  config.h.in settings
#-------------------------------------
set(SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG)
set(LOG_FILE_NAME "appliaction.log")


configure_file(
	"./cmake/config.h.in" 
	"${PROJECT_SOURCE_DIR}/include/config.h"
	@ONLY
)


######################################
#  Information
######################################
message(" ===================== Information ======================")
message(CMAKE_SOURCE_DIR         	    =  ${CMAKE_SOURCE_DIR})
message(PROJECT_SOURCE_DIR            =  ${PROJECT_SOURCE_DIR})
message(CMAKE_INCLUDE_PATH            =  ${CMAKE_INCLUDE_PATH})
message(CMAKE_LIBRARY_PATH            =  ${CMAKE_LIBRARY_PATH})
message(PROJECT_SOURCE_DIR            =  ${PROJECT_SOURCE_DIR})
message(CMAKE_BUILD_TYPE              =  ${CMAKE_BUILD_TYPE})

message(" ===================== SETTING ======================")



message(" ==================== LIBRARY PATH =====================")
message(CMAKE_THREAD_LIBS_INIT		    =  ${CMAKE_THREAD_LIBS_INIT})
message(CMAKE_DL_LIBS	         			  =  ${CMAKE_DL_LIBS})
message(WS2_LIBRARY                   =  ${WS2_LIBRARY})




message(" ==================== INCLUDE PATH =====================")
message(BOTAN_INCLUDE_DIR             =  ${BOTAN_INCLUDE_DIR})




# Add source to this project's executable.
add_executable (${PROJECT_NAME}   
  "src/main.cpp" 
 "src/pch.cpp" "src/cmdParser.cpp" "src/Client.cpp" "src/Server.cpp" "src/UdpConnection.cpp")


target_include_directories( ${PROJECT_NAME}  
	PRIVATE 
		${PROJECT_SOURCE_DIR}/include
)


#spdlog_enable_warnings(${PROJECT_NAME})


target_link_libraries( ${PROJECT_NAME}  
	PRIVATE 
		${WS2_LIBRARY}
		${CMAKE_THREAD_LIBS_INIT}
		${CMAKE_DL_LIBS}
)





