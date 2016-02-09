CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(MyProject)

#SET(CMAKE_INCLUDE_CURRENT_DIR ON)

#ADD_SUBDIRECTORY(src)

set(CMAKE_CXX_FLAGS "-g -Wall")


FIND_PACKAGE(PkgConfig)
#PKG_CHECK_MODULES(GLIB2 REQUIRED glib-2.0>=2.36.0)

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(MacroOptionalFindPackage)
#macro_optional_find_package (Glib)
#macro_optional_find_package (NCurses)

#INCLUDE_DIRECTORIES(
#    Glib_INCLUDE_DIRS
#)
ADD_DEFINITIONS(-std=gnu99)

find_package(Curses REQUIRED)
find_package(Glib REQUIRED)
#find_package(menu REQUIRED)

include_directories(${Glib_INCLUDE_DIRS} ${CURSES_INCLUDE_DIR})


add_executable(cdemo src/main.c src/scan.c src/settings.c src/term.c src/utils.c src/utils_string.c)

target_link_libraries(cdemo ${Glib_LIBRARIES} ${CURSES_LIBRARIES} -lmenu)



#SET(MY_SRC
#    fileA.cpp
#    fileB.cpp
#    ...
#)



#file(GLOB MY_SRC
#    "src/*.h"
#    "src/*.c"
#)

#ADD_EXECUTABLE(myapp
#    ${MY_SRC}
#)


#ADD_LIBRARY(mylib STATIC
#    ${MY_LIBSRCS}
#)

#TARGET_LINK_LIBRARIES(mylib
#    GLIB2_LIBRARIES
#)
#INCLUDE_DIRECTORIES(
#    GLIB2_INCLUDE_DIRS
#)