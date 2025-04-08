cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(ALPR)

macro(use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    endif ()
  else ()
    set (CMAKE_CXX_STANDARD 17)
  endif ()
endmacro(use_cxx11)
use_cxx11()
find_package(OpenCV REQUIRED)                              

set(SOURCES src/main.cpp src/RTSPcam.cpp src/Regression.cpp src/Tjson.cpp src/MJPG_sender.cpp src/Regression.cpp)

set(EXTRA_LIBS ${OpenCV_LIBS} /usr/local/lib/libdarknet.so)

set(EXECUTABLE_OUTPUT_PATH "../")

include_directories(include ${OpenCV_INCLUDE_DIRS} ./include /usr/local/include )

add_link_options(-fopenmp -s -ldl -lpthread -O3 -lgomp -rdynamic )

add_executable(ALPR ${SOURCES})

target_link_libraries(ALPR ${EXTRA_LIBS})


#// sample line added for testing amke and build
