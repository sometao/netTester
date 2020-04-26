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
#  zlib for httplib gzip.
#-------------------------------------
#set(ZLIB_LIBRARY "${PROJECT_SOURCE_DIR}/lib/zlibwapi.lib")
if(WIN32)
	find_library(ZLIB_LIBRARY zlibwapi.lib)
elseif(UNIX)
	find_library(ZLIB_LIBRARY libz.a)
endif()


#-------------------------------------
#  openSSL for httplib https.
#-------------------------------------
#set(LIBSSL_LIBRARY "${PROJECT_SOURCE_DIR}/lib/libssl.lib")
#set(LIBCRYPTO_LIBRARY "${PROJECT_SOURCE_DIR}/lib/libcrypto.lib")
if(WIN32)
	find_library(LIBSSL_LIBRARY libssl.lib)
	find_library(LIBCRYPTO_LIBRARY libcrypto.lib)
elseif(UNIX)
	find_library(LIBSSL_LIBRARY libssl.a)
	find_library(LIBCRYPTO_LIBRARY libcrypto.a)
endif()





#-------------------------------------
#  sqlite3 lib.
#-------------------------------------
#set(SQLITE3_LIBRARY "${PROJECT_SOURCE_DIR}/lib/sqlite3.lib")
if(WIN32)
	find_library(SQLITE3_LIBRARY sqlite3.lib REQUIRED)
elseif(UNIX)
	find_library(SQLITE3_LIBRARY libsqlite3.a REQUIRED)
endif()


#-------------------------------------
#  botan lib.
#-------------------------------------
#set(BOTAN_LIBRARY "${PROJECT_SOURCE_DIR}/lib/botan.lib")

find_path(BOTAN_INCLUDE_DIR "botan/botan.h" REQUIRED)
if(WIN32)
	find_library(BOTAN_LIBRARY botan.lib REQUIRED)
elseif(UNIX)
	find_library(BOTAN_LIBRARY libbotan-2.a REQUIRED)
endif()


#-------------------------------------
#  PRE BUILD DEFINE
#-------------------------------------
if(WIN32)
	add_definitions(-DZLIB_WINAPI )
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

set(CPPHTTPLIB_THREAD_POOL_COUNT 64)
option(CPPHTTPLIB_ZLIB_SUPPORT "httplib gzip support" OFF)
option(CPPHTTPLIB_OPENSSL_SUPPORT "httplib ssl support" ON)

set(SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE)
set(LOG_FILE_NAME "appliaction.log")
set(CA_CERT_FILE "./resources/ca-bundle.crt")
set(SQLITE_DB_FILE "${PROJECT_SOURCE_DIR}/appliaction.db")

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
message(CPPHTTPLIB_ZLIB_SUPPORT       =  ${CPPHTTPLIB_ZLIB_SUPPORT})
message(CPPHTTPLIB_THREAD_POOL_COUNT  =  ${CPPHTTPLIB_THREAD_POOL_COUNT})


message(" ==================== LIBRARY PATH =====================")
message(CMAKE_THREAD_LIBS_INIT		  =  ${CMAKE_THREAD_LIBS_INIT})
message(CMAKE_DL_LIBS	         	  =  ${CMAKE_DL_LIBS})
message(LIBSSL_LIBRARY                =  ${LIBSSL_LIBRARY})
message(LIBCRYPTO_LIBRARY             =  ${LIBCRYPTO_LIBRARY})
message(SQLITE3_LIBRARY               =  ${SQLITE3_LIBRARY})
message(ZLIB_LIBRARY                  =  ${ZLIB_LIBRARY})
message(BOTAN_LIBRARY                 =  ${BOTAN_LIBRARY})




message(" ==================== INCLUDE PATH =====================")
message(BOTAN_INCLUDE_DIR             =  ${BOTAN_INCLUDE_DIR})




# Add source to this project's executable.
add_executable (${PROJECT_NAME}   
  "src/main.cpp" 
 "src/pch.cpp" "src/cmdParser.cpp")


target_include_directories( ${PROJECT_NAME}  
	PRIVATE 
		${PROJECT_SOURCE_DIR}/include
		${BOTAN_INCLUDE_DIR}
)


#spdlog_enable_warnings(${PROJECT_NAME})


target_link_libraries( ${PROJECT_NAME}  
	PRIVATE 
		${ZLIB_LIBRARY} 
		${LIBSSL_LIBRARY} 
		${ZLIB_LIBRARY} 
		${LIBCRYPTO_LIBRARY} 
		${SQLITE3_LIBRARY} 
		${BOTAN_LIBRARY} 
		${CMAKE_THREAD_LIBS_INIT}
		${CMAKE_DL_LIBS}
)


if(WIN32)
    # Copy DLL files to output directory
	message("COPY DLLs TO target path.")
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${PROJECT_SOURCE_DIR}/lib/zlibwapi.dll"              
			$<TARGET_FILE_DIR:${PROJECT_NAME}>
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${PROJECT_SOURCE_DIR}/lib/libcrypto-1_1-x64.dll"              
			$<TARGET_FILE_DIR:${PROJECT_NAME}>		
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${PROJECT_SOURCE_DIR}/lib/libssl-1_1-x64.dll"              
			$<TARGET_FILE_DIR:${PROJECT_NAME}>
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${PROJECT_SOURCE_DIR}/lib/sqlite3.dll"              
			$<TARGET_FILE_DIR:${PROJECT_NAME}>
	)


endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD  
	COMMAND ${CMAKE_COMMAND} -E copy_directory 
		"${PROJECT_SOURCE_DIR}/resources"              
		"$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
)



