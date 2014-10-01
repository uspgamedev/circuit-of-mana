
if (CMAKE_COMPILER_IS_GNUCC)
  add_definitions (-std=gnu++11)
endif ()

add_ugdk_executable(circuit-of-mana main.cc)
