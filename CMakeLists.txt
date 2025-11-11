cmake_minimum_required(VERSION 3.10)

project(ifs_fractals_generator)

add_executable(main)

target_sources(main PRIVATE ${PROJECT_SOURCE_DIR}/src/glad.c ${PROJECT_SOURCE_DIR}/src/input.cpp ${PROJECT_SOURCE_DIR}/src/shaders.cpp 
${PROJECT_SOURCE_DIR}/src/renderer.cpp ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_include_directories(main PRIVATE ${PROJECT_SOURCE_DIR}/Libraries/include)
add_library(glfw STATIC IMPORTED)
add_library(glm STATIC IMPORTED)

if(CMAKE_SYSTEM MATCHES Linux)
message(Linux Detected!)
set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/Libraries/lib/libglfw3.a)
endif()

if(CMAKE_SYSTEM MATCHES Windows)
message(Windows Detected!)
set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/Libraries/lib/glfw3.lib)
set_target_properties(glm PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/Libraries/lib/glm.lib)
endif()

target_link_libraries(main PRIVATE glfw glm)
