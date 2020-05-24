# This file described build process for xstarter
# Requires Curses and Glib

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(xstarter)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

set(PROJECT_VERSION "0.8.0")
if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set(CMAKE_C_FLAGS "-g -Wall -pedantic")
else()
  set(CMAKE_C_FLAGS "-Wall -pedantic -O3")
endif()

# cpack
set(CPACK_PACKAGE_CONTACT "Maciej Lechowski")
set(CPACK_GENERATOR "DEB;RPM;ZIP;TGZ")
set(CPACK_PACKAGE_DESCRIPTION, "Application launcher for Linux")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

# deb stuff
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://lchsk.com/xstarter")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libncurses5 (>= 5.0), libglib2.0-0 (>= 2.0)")

# rpm stuff
set(CPACK_RPM_PACKAGE_LICENSE "GPL")
set(CPACK_RPM_PACKAGE_URL "https://lchsk.com/xstarter")
set(CPACK_RPM_PACKAGE_DESCRIPTION "Application launcher for Linux")
set(CPACK_RPM_PACKAGE_REQUIRES "ncurses >= 5.0, glib2 >= 2.0")

include(CPack)

# end cpack

add_definitions(-std=c99)

find_package(PkgConfig REQUIRED)

message(STATUS "Cmake build type: " ${CMAKE_BUILD_TYPE})
message(STATUS "Curses include directory: " ${CURSES_INCLUDE_DIR})

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(CURSES REQUIRED ncurses)

include_directories(
  ${GLIB_INCLUDE_DIRS}
  ${CURSES_INCLUDE_DIRS}
)

add_executable(xstarter
  src/main.c
  src/scan.c
  src/settings.c
  src/term.c
  src/utils.c
  src/utils_string.c
  xstarter.1
)

target_link_libraries(xstarter
  ${GLIB_LIBRARIES}
  ${CURSES_LIBRARIES}
  -pthread
)

install(TARGETS xstarter DESTINATION bin)
install(FILES xstarter.1 DESTINATION share/man/man1)
