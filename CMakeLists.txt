cmake_minimum_required(VERSION 3.10)

project(DragonTea VERSION 100 LANGUAGES C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(gtk3ui REQUIRED gtk+-3.0)
pkg_check_modules(libcurl REQUIRED libcurl)
pkg_check_modules(json REQUIRED json-c)
pkg_check_modules(gthread REQUIRED gthread-2.0)

file(GLOB_RECURSE SOURCE "${CMAKE_SOURCE_DIR}/" "*.c" "*.h")

add_executable(msg-app ${SOURCE})
target_compile_definitions(msg-app PUBLIC TEA_APP_VERSION=${PROJECT_VERSION})
target_include_directories(msg-app PUBLIC "${CMAKE_SOURCE_DIR}/include/" ${gtk3ui_INCLUDE_DIRS} ${gthread_INCLUDE_DIRS}
                                           ${libcurl_INCLUDE_DIRS} ${json_INCLUDE_DIRS})
target_link_libraries(msg-app ${gtk3ui_LIBRARIES} ${libcurl_LIBRARIES} ${json_LIBRARIES} ${gthread_LIBRARIES})
