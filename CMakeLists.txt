cmake_minimum_required(VERSION 3.10)

project(ifs_fractals_generator)

add_executable(main)

set_property(TARGET main PROPERTY CXX_STANDARD 17)
set_property(TARGET main PROPERTY CXX_STANDARD_REQUIRED ON)

target_sources(main PRIVATE ${PROJECT_SOURCE_DIR}/src/glad.c ${PROJECT_SOURCE_DIR}/src/imgui/imgui.cpp ${PROJECT_SOURCE_DIR}/src/imgui/imgui_draw.cpp
${PROJECT_SOURCE_DIR}/src/imgui/imgui_impl_glfw.cpp ${PROJECT_SOURCE_DIR}/src/imgui/imgui_impl_opengl3.cpp
${PROJECT_SOURCE_DIR}/src/imgui/imgui_tables.cpp ${PROJECT_SOURCE_DIR}/src/imgui/imgui_widgets.cpp
${PROJECT_SOURCE_DIR}/src/ifs.cpp ${PROJECT_SOURCE_DIR}/src/input.cpp ${PROJECT_SOURCE_DIR}/src/shaders.cpp 
${PROJECT_SOURCE_DIR}/src/renderer.cpp ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_include_directories(main PRIVATE ${PROJECT_SOURCE_DIR}/Libraries/include ${PROJECT_SOURCE_DIR}/Libraries/include/imgui)
add_library(glfw STATIC IMPORTED)
add_library(glm STATIC IMPORTED)

if(CMAKE_SYSTEM MATCHES Linux)
message(Linux Detected!)
set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/Libraries/lib/libglfw3.a)
set_target_properties(glm PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/Libraries/lib/libglm.a)
endif()

if(CMAKE_SYSTEM MATCHES Windows)
message(Windows Detected!)
set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/Libraries/lib/glfw3.lib)
set_target_properties(glm PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/Libraries/lib/glm.lib)
endif()

target_link_libraries(main PRIVATE glfw glm)
