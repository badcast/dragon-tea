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

add_executable(dragontea)

# COMPILE BUILTIN SERVERS
set(BUILTIN_TEMPLATE_FILE "${CMAKE_BINARY_DIR}/builtin_template.c")

file(GLOB BUILTIN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/builtin/*.url")
list(LENGTH BUILTIN_FILES BF_LENGTH)

if(NOT EXISTS "${BUILTIN_TEMPLATE_FILE}")
  if(BF_LENGTH GREATER 0)
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/builtin/builtin_template.c"
         DESTINATION ${CMAKE_BINARY_DIR})

    foreach(F ${BUILTIN_FILES})
      file(READ ${F} F_CONTENT)
      string(STRIP ${F_CONTENT} F_CONTENT)
      file(APPEND ${BUILTIN_TEMPLATE_FILE} "\"${F_CONTENT}\",\n")
    endforeach()
    file(APPEND ${BUILTIN_TEMPLATE_FILE} "};")
  endif()
endif()

if(EXISTS "${BUILTIN_TEMPLATE_FILE}")
  list(APPEND DRAGONTEA_SOURCES ${BUILTIN_TEMPLATE_FILE})
  target_compile_definitions(dragontea PUBLIC BUILTIN_SERVERS=ON
                                              BUILTIN_SERVERS_N=${BF_LENGTH})
endif()

target_sources(dragontea PUBLIC ${DRAGONTEA_SOURCES})

target_include_directories(
  dragontea
  PUBLIC "${CMAKE_SOURCE_DIR}/include/" ${gtk3ui_INCLUDE_DIRS}
         ${gthread_INCLUDE_DIRS} ${libcurl_INCLUDE_DIRS} ${json_INCLUDE_DIRS})

target_link_libraries(dragontea ${gtk3ui_LIBRARIES} ${libcurl_LIBRARIES}
                      ${json_LIBRARIES} ${gthread_LIBRARIES})

target_compile_definitions(dragontea
                           PUBLIC DRAGON_TEA_VERSION="${DRAGON_TEA_VERSION}")

if(UNIX OR LINUX)
  set(LOCALE_DIR "${CMAKE_INSTALL_PREFIX}/share/locale")
  target_compile_definitions(dragontea
                             PUBLIC DRAGON_TEA_LOCALE_DIR="${LOCALE_DIR}")

  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/localization/locale"
          DESTINATION "${CMAKE_INSTALL_PREFIX}/share")

  install(FILES "${CMAKE_BINARY_DIR}/dragontea"
          DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/")
endif()
