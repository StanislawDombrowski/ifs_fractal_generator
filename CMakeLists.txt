cmake_minimum_required(VERSION 3.10)

project(ifs_fractals_generator)

add_executable(main)

file(COPY ${CMAKE_SOURCE_DIR}/transformations
     DESTINATION ${CMAKE_BINARY_DIR})

file(COPY ${CMAKE_SOURCE_DIR}/src/shaders
     DESTINATION ${CMAKE_BINARY_DIR})

add_custom_command(TARGET main POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/transformations
          $<TARGET_FILE_DIR:main>/transformations)

add_custom_command(TARGET main POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/src/shaders
          $<TARGET_FILE_DIR:main>/src/shaders)

set_property(TARGET main PROPERTY CXX_STANDARD 17)
set_property(TARGET main PROPERTY CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)

add_compile_options(-g -O0 -fno-omit-frame-pointer -fsanitize=address,undefined)
add_link_options(-fsanitize=address,undefined)

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
find_package(X11 REQUIRED)

message(STATUS "X11_FOUND = ${X11_FOUND}")
message(STATUS "X11_INCLUDE_DIR = ${X11_INCLUDE_DIR}")
message(STATUS "X11_LIBRARIES = ${X11_LIBRARIES}")

  if(TARGET X11::X11)
    target_link_libraries(main PRIVATE X11::X11)
    if(TARGET X11::Xext)
      target_link_libraries(main PRIVATE X11::Xext)
    endif()
    if(TARGET X11::ICE)
      target_link_libraries(main PRIVATE X11::ICE)
    endif()
    if(TARGET X11::SM)
      target_link_libraries(main PRIVATE X11::SM)
    endif()
  else()
    # Fallback: variables from FindX11
    target_link_libraries(main PRIVATE ${X11_LIBRARIES})
    target_include_directories(main PRIVATE ${X11_INCLUDE_DIR})
  endif()

set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/Libraries/lib/libglfw3.a)
set_target_properties(glm PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/Libraries/lib/libglm.a)
endif()

if(CMAKE_SYSTEM MATCHES Windows)
message(Windows Detected!)
set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/Libraries/lib/glfw3.lib)
set_target_properties(glm PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/Libraries/lib/glm.lib)
endif()

target_link_libraries(main PRIVATE glfw glm)
