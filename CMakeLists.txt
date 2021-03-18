## set target project
file(GLOB target_src "*.h" "*.cpp") # look for source files
file(GLOB target_shaders "shaders/*.vert" "shaders/*.frag") # look for shaders
add_executable(${subdir} ${target_src} ${target_shaders} shaders/portal.cpp shaders/portal.h Portal.cpp Portal.h)

## set link libraries
target_link_libraries(${subdir} ${libraries})

## add local source directory to include paths
target_include_directories(${subdir} PUBLIC ../portal_project)

## copy shaders folder to build folder
file(COPY ../portal_project/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ../portal_project/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

## copy again at the time the current target gets compiled
add_custom_command(
        TARGET ${subdir} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ../portal_project/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders
        COMMENT "Copying shaders" VERBATIM
)