
cmake_minimum_required(VERSION 2.8.11)
project(circuit-of-mana)

find_package(Git)
if (NOT GIT_FOUND)
  message (FATAL_ERROR "This project requires Git!")
endif (NOT GIT_FOUND)

if (EXISTS ${CMAKE_SOURCE_DIR}/local/ugdk)
  message (STATUS "Found local ugdk!")
  set (UGDK_SUBDIR ${CMAKE_SOURCE_DIR}/local/ugdk)
else (EXISTS ${CMAKE_SOURCE_DIR}/local/ugdk)
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/externals/ugdk/CMakeLists.txt")
      execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init
                          WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
  endif()
  set (UGDK_SUBDIR externals/ugdk)
endif ()

add_custom_target (
  assets
  COMMAND ${CMAKE_COMMAND} -E make_directory
          ${CMAKE_CURRENT_BINARY_DIR}/src/assets        
  COMMAND ${GIT_EXECUTABLE} archive
          --remote=git://uspgamedev.org/circuit-resources.git
          -o ${CMAKE_CURRENT_BINARY_DIR}/resources.tar.gz
          master assets/
  COMMAND ${CMAKE_COMMAND} -E tar xf ${CMAKE_CURRENT_BINARY_DIR}/resources.tar.gz
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src/
  COMMAND ${CMAKE_COMMAND} -E remove
          ${CMAKE_CURRENT_BINARY_DIR}/resources.tar.gz
)

add_subdirectory(${UGDK_SUBDIR})

CONFIGURE_FILE(
    "doxystuff/Doxyfile.in"
    "doxystuff/Doxyfile"
)

configure_file(cmake/plugins.cfg src/assets/plugins.cfg)
configure_file(cmake/plugins_d.cfg src/assets/plugins_d.cfg)
configure_file(cmake/resources.cfg src/assets/resources.cfg)
configure_file(cmake/resources_d.cfg src/assets/resources_d.cfg)

add_subdirectory(src)


