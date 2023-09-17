cmake_minimum_required(VERSION 3.10)

# READ VERSION
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" DRAGON_TEA_VERSION)

string(STRIP "${DRAGON_TEA_VERSION}" DRAGON_TEA_VERSION)

project(
  DragonTea
  VERSION ${DRAGON_TEA_VERSION}
  LANGUAGES C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(gtk3ui REQUIRED gtk+-3.0)
pkg_check_modules(libcurl REQUIRED libcurl)
pkg_check_modules(json REQUIRED json-c)
pkg_check_modules(gthread REQUIRED gthread-2.0)

file(GLOB_RECURSE DRAGONTEA_SOURCES "${CMAKE_SOURCE_DIR}/src/*.c"
     "${CMAKE_SOURCE_DIR}/include/*.h")

add_executable(dragontea ${DRAGONTEA_SOURCES})
target_compile_definitions(dragontea
                           PUBLIC DRAGON_TEA_VERSION="${DRAGON_TEA_VERSION}")
target_include_directories(
  dragontea
  PUBLIC "${CMAKE_SOURCE_DIR}/include/" ${gtk3ui_INCLUDE_DIRS}
         ${gthread_INCLUDE_DIRS} ${libcurl_INCLUDE_DIRS} ${json_INCLUDE_DIRS})
target_link_libraries(dragontea ${gtk3ui_LIBRARIES} ${libcurl_LIBRARIES}
                      ${json_LIBRARIES} ${gthread_LIBRARIES})
