include_directories(${SDL2_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/world)

add_executable(one_more_doom main.cpp sdl_base_class.cpp sdl_window.cpp app_class.cpp)

target_link_libraries(one_more_doom ${SDL2_LIBRARIES} SDL2_ttf world mobjs utils graph renderer)
