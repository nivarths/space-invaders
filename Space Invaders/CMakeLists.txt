cmake_minimum_required(VERSION 3.10)
project(SpaceInvaders)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find GLFW package
find_package(glfw3 REQUIRED)

# Add source files
set(SOURCES
    main.cpp
    Game.cpp
    Entity.cpp
    Sprite.cpp
    Player.cpp
    Alien.cpp
    Bullet.cpp
)

# Add header files
set(HEADERS
    Game.h
    Entity.h
    Sprite.h
    Player.h
    Alien.h
    Bullet.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link GLFW
target_link_libraries(${PROJECT_NAME} glfw)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    C:/Libraries/vcpkg/installed/x86-windows/include
) 