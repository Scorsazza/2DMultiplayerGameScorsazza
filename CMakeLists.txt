cmake_minimum_required(VERSION 3.14)
project(SFMLGame)

set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    set(SFML_STATIC_LIBRARIES TRUE)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

# set(CMAKE_PREFIX_PATH C:/)
find_package(SFML 2.5.1 COMPONENTS system window graphics network audio)

set(SOURCE_FILES src/main.cpp src/Game.cpp src/Game.h src/Server.cpp src/Server.h src/Client.cpp src/Client.h src/Server.cpp src/Server.h src/Player.cpp src/Player.h src/tinyxml2.h src/tinyxml2.cpp src/Tile.cpp src/Tile.h)
add_executable(SFMLGame src/Tile.cpp src/Tile.h src/FreeFuncs.cpp
        src/ImageLayer.cpp src/LayerGroup.cpp src/Map.cpp src/miniz.c src/miniz.h
        src/Object.cpp src/ObjectGroup.cpp src/ObjectTypes.cpp src/Property.cpp
        src/TileLayer.cpp src/Tileset.cpp src/detail/pugixml.cpp          ${SOURCE_FILES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules")

target_link_libraries (SFMLGame sfml-graphics sfml-window sfml-system sfml-network sfml-audio)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Data/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data/Map DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Data/Map/)