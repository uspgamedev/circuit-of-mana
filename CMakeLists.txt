
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
  COMMAND ${CMAKE_COMMAND} -E remove_directory
          ${CMAKE_CURRENT_BINARY_DIR}/assets/spritesheets
  COMMAND ${GIT_EXECUTABLE} archive
          --remote=git://uspgamedev.org/circuit-ugdk-resources.git
          -o ${CMAKE_CURRENT_BINARY_DIR}/assets/resources.tar.gz
          master resources/
  COMMAND ${CMAKE_COMMAND} -E tar xf ${CMAKE_CURRENT_BINARY_DIR}/assets/resources.tar.gz
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/assets/
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_CURRENT_BINARY_DIR}/assets/resources/spritesheets/
          ${CMAKE_CURRENT_BINARY_DIR}/assets/spritesheets/
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_CURRENT_BINARY_DIR}/assets/resources/maps/
          ${CMAKE_CURRENT_BINARY_DIR}/assets/maps/
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_CURRENT_BINARY_DIR}/assets/resources/fonts/
          ${CMAKE_CURRENT_BINARY_DIR}/assets/fonts/
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_CURRENT_BINARY_DIR}/assets/resources/animations/
          ${CMAKE_CURRENT_BINARY_DIR}/assets/animations/
  COMMAND ${CMAKE_COMMAND} -E remove_directory
          ${CMAKE_CURRENT_BINARY_DIR}/assets/resources/
  COMMAND ${CMAKE_COMMAND} -E remove
          ${CMAKE_CURRENT_BINARY_DIR}/assets/resources.tar.gz
)

add_subdirectory(${UGDK_SUBDIR})

add_subdirectory(src)


