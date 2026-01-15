cmake_minimum_required(VERSION 3.21)

project(Creature_Sim_cpp
  VERSION 0.1.0
  LANGUAGES CXX
)

# --------------------------
# Global C++ settings
# --------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (MSVC)
  add_compile_options(/EHsc)
endif()

# --------------------------
# Qt settings
# --------------------------
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Charts Multimedia)


# --------------------------
# Main simulation library
# --------------------------
add_library(CreatureSimLib
  SimRandom.cpp
  SimEnvironment.cpp
  SimFood.cpp
  SimCreature.cpp
  SimBehavior.cpp
)

target_include_directories(CreatureSimLib PUBLIC
  ${PROJECT_SOURCE_DIR}
)

target_link_libraries(CreatureSimLib PUBLIC
  Qt6::Core
  Qt6::Charts
  Qt6::Multimedia
)

# --------------------------
# Testing
# --------------------------
enable_testing()
add_subdirectory(tests)