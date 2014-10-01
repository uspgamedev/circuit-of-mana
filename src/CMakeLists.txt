
if (CMAKE_COMPILER_IS_GNUCC)
  add_definitions (-std=gnu++11)
endif ()

add_ugdk_executable(circuit-of-mana main.cc)
set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -lXt -ldl")

