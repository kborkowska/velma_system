cmake_minimum_required(VERSION 2.8.3)
project(velma_data_conversions)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})

### ROS Dependencies ###
# Find the RTT-ROS package (this transitively includes the Orocos CMake macros)
find_package(catkin REQUIRED COMPONENTS
  rtt_ros
  eigen_conversions
  subsystem_common
)

find_package(OROCOS-RTT REQUIRED COMPONENTS rtt-scripting rtt-transport-corba)
include(${OROCOS-RTT_USE_FILE_PATH}/UseOROCOS-RTT.cmake )

find_package(Eigen3 REQUIRED)

include_directories(${catkin_INCLUDE_DIRS} ${Eigen3_INCLUDE_DIR} ${USE_OROCOS_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ADD_DEFINITIONS(-DEIGEN_RUNTIME_NO_MALLOC)

#include_directories(${USE_OROCOS_INCLUDE_DIRS})
### Orocos Targets ###

orocos_component(velma_data_conversions
  src/eigen_vector_1.cpp
  src/eigen_vector_2.cpp
  src/eigen_vector_4.cpp
  src/eigen_vector_7.cpp
  src/eigen_vector_8.cpp
  src/eigen_vector_15.cpp
  src/eigen_vector_33.cpp
  src/eigen_matrix77.cpp
  src/double_int.cpp
  src/velma_data_conversions.cpp
)
target_link_libraries(velma_data_conversions ${catkin_LIBRARIES} ${USE_OROCOS_LIBRARIES})

### Orocos Package Exports and Install Targets ###

# Export package information (replaces catkin_package() macro) 
orocos_generate_package(
  DEPENDS rtt_ros
)

