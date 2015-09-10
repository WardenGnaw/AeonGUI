# Copyright 2015 AeonGames, Rodrigo Hernandez
# Licensed under the terms of the MIT License.
if(NOT GMOCK_FOUND)
    # Download package.
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/gmock-1.7.0.zip")
        message(STATUS "Please wait while downloading gmock...")
        set(ENV{https_proxy} "${HTTP_PROXY}")
        file(DOWNLOAD "https://googlemock.googlecode.com/files/gmock-1.7.0.zip" "${CMAKE_SOURCE_DIR}/gmock-1.7.0.zip" STATUS gmock_dl_status LOG GMOCK_dl_log SHOW_PROGRESS)
        if(NOT gmock_dl_status MATCHES "0;\"No error\"")
            file(REMOVE "${CMAKE_SOURCE_DIR}/gmock-1.7.0")
            message(FATAL_ERROR "Download failed, did you set a proxy? STATUS: ${gmock_dl_status}")
			file(REMOVE "${CMAKE_SOURCE_DIR}/gmock-1.7.0.zip")
        endif(NOT gmock_dl_status MATCHES "0;\"No error\"")
        message(STATUS "Done downloading gmock binary")
    endif(NOT EXISTS "${CMAKE_SOURCE_DIR}/gmock-1.7.0.zip")
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/gmock-1.7.0")
        # Extract gmock.
        message(STATUS "Extracting gmock...")
        execute_process(COMMAND cmake -E tar xzvf gmock-1.7.0.zip WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} ERROR_VARIABLE extract_result)
    endif(NOT EXISTS "${CMAKE_SOURCE_DIR}/gmock-1.7.0")
	set(gtest_force_shared_crt ON CACHE BOOL "Forced value for gtest" FORCE)
	set(GTEST_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/gmock-1.7.0/gtest/include" CACHE PATH "GTest Include path" FORCE)
	set(GMOCK_LIBRARIES gmock)
	set(GMOCK_MAIN_LIBRARIES gmock_main)
	set(GMOCK_BOTH_LIBRARIES ${GMOCK_LIBRARIES} ${GMOCK_MAIN_LIBRARIES})
	set(GMOCK_LIBRARY gmock CACHE PATH "gmock Library" FORCE)
	set(GMOCK_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/gmock-1.7.0/include" CACHE PATH "gmock Include path" FORCE)
	set(GMOCK_MAIN_LIBRARY GMOCK_main CACHE PATH "gmock Main Function Library" FORCE)
	set(GMOCK_SUBDIRECTORY ${CMAKE_SOURCE_DIR}/gmock-1.7.0 CACHE PATH "gmock Source Path" FORCE)
endif()
if(DEFINED GMOCK_SUBDIRECTORY)
    add_subdirectory(${GMOCK_SUBDIRECTORY})
endif()