cmake_minimum_required(VERSION 3.20)
project(SpaceShooterGame CXX)

set(CMAKE_CXX_STANDARD 17)

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window audio system REQUIRED)

# Include directories
include_directories(${SFML_INCLUDE_DIRS})

# Source files
file(GLOB SOURCES "src/*.cpp" "src/*.h")

# Executable
add_executable(SpaceShooter ${SOURCES})

# Link libraries
target_link_libraries(SpaceShooter 
    ${SFML_LIBRARIES}
    ${SFML_DEPENDENCIES}
)

# Output directory
set_target_properties(SpaceShooter PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
