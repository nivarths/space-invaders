// Compile with C++14 or later: -std=c++14
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstdint>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <memory>
#include "Buffer.h"
#include "Sprite.h"
#include "Game.h"
#include "AlienType.h"
#include "SpriteAnimation.h"
#include "Entity.h"
#include "Player.h"
#include "Alien.h"
#include "Bullet.h"
#include "GameConfig.h"
#include "shaders.h"
#include <stdexcept>
#include "Font.h"

extern std::unique_ptr<Game> game;

// Function declarations
void error_callback(int error, const char* description);
uint32_t rgb_to_uint32(uint8_t r, uint8_t g, uint8_t b);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void initialize_sprites();
GLuint create_shader_program();

// Global Variables
std::unique_ptr<Game> game;
Sprite* player_sprite = nullptr;
std::vector<Sprite*> alien_sprites;
Sprite* bullet_sprite = nullptr;
std::vector<SpriteAnimation*> alien_animations;
int player_move_dir = 1; // initializing the player movement to right
bool game_running = false; // global variable to check if game is running
int move_dir = 0; // variable to indicate movement direction
bool fire_pressed = false; // variable to indicate if the projectile was fired
Sprite* heartSprite = nullptr;

// Constants for classic Space Invaders dimensions
const size_t BUFFER_WIDTH = 224;
const size_t BUFFER_HEIGHT = 256;
// Window size is scaled up for better visibility
const size_t WINDOW_WIDTH = BUFFER_WIDTH * 2;  // 448
const size_t WINDOW_HEIGHT = BUFFER_HEIGHT * 2;  // 512

void initialize_sprites() {
    alien_sprites.clear();

    // TYPE_A - small squid-like alien
    uint8_t* alien_data_A1 = new uint8_t[64] {
        0,0,0,1,1,0,0,0, // ...@@...
        0,0,1,1,1,1,0,0, // ..@@@@..
        0,1,1,1,1,1,1,0, // .@@@@@@.
        1,1,0,1,1,0,1,1, // @@.@@.@@
        1,1,1,1,1,1,1,1, // @@@@@@@@
        0,1,0,1,1,0,1,0, // .@.@@.@.
        1,0,0,0,0,0,0,1, // @......@
        0,1,0,0,0,0,1,0  // .@....@.
    };
    alien_sprites.push_back(new Sprite(8, 8, alien_data_A1));

    uint8_t* alien_data_A2 = new uint8_t[64] {
        0,0,0,1,1,0,0,0, // ...@@...
        0,0,1,1,1,1,0,0, // ..@@@@..
        0,1,1,1,1,1,1,0, // .@@@@@@.
        1,1,0,1,1,0,1,1, // @@.@@.@@
        1,1,1,1,1,1,1,1, // @@@@@@@@
        0,0,1,0,0,1,0,0, // ..@..@..
        0,1,0,1,1,0,1,0, // .@.@@.@.
        1,0,1,0,0,1,0,1  // @.@..@.@
    };
    alien_sprites.push_back(new Sprite(8, 8, alien_data_A2));

    // TYPE_B - mid-size crab-like alien
    uint8_t* alien_data_B1 = new uint8_t[88] {
        0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
        0,0,0,1,0,0,0,1,0,0,0, // ...@...@...
        0,0,1,1,1,1,1,1,1,0,0, // ..@@@@@@@..
        0,1,1,0,1,1,1,0,1,1,0, // .@@.@@@.@@.
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
        1,0,1,0,0,0,0,0,1,0,1, // @.@.....@.@
        0,0,0,1,1,0,1,1,0,0,0  // ...@@.@@...
    };
    alien_sprites.push_back(new Sprite(11, 8, alien_data_B1));

    uint8_t* alien_data_B2 = new uint8_t[88] {
        0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
        1,0,0,1,0,0,0,1,0,0,1, // @..@...@..@
        1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
        1,1,1,0,1,1,1,0,1,1,1, // @@@.@@@.@@@
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
        0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
        0,1,0,0,0,0,0,0,0,1,0  // .@.......@.
    };
    alien_sprites.push_back(new Sprite(11, 8, alien_data_B2));

    // TYPE_C - large beetle-like alien
    uint8_t* alien_data_C1 = new uint8_t[96] {
        0,0,0,0,1,1,1,1,0,0,0,0, // ....@@@@....
        0,1,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@@.
        1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
        1,1,1,0,0,1,1,0,0,1,1,1, // @@@..@@..@@@
        1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
        0,0,0,1,1,0,0,1,1,0,0,0, // ...@@..@@...
        0,0,1,1,0,1,1,0,1,1,0,0, // ..@@.@@.@@..
        1,1,0,0,0,0,0,0,0,0,1,1  // @@........@@
    };
    alien_sprites.push_back(new Sprite(12, 8, alien_data_C1));

    uint8_t* alien_data_C2 = new uint8_t[96] {
        0,0,0,0,1,1,1,1,0,0,0,0, // ....@@@@....
        0,1,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@@.
        1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
        1,1,1,0,0,1,1,0,0,1,1,1, // @@@..@@..@@@
        1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
        0,0,1,1,1,0,0,1,1,1,0,0, // ..@@@..@@@..
        0,1,1,0,0,1,1,0,0,1,1,0, // .@@..@@..@@.
        0,0,1,1,0,0,0,0,1,1,0,0  // ..@@....@@..
    };
    alien_sprites.push_back(new Sprite(12, 8, alien_data_C2));

    // Player sprite (make sure it's right side up)
    uint8_t* player_data = new uint8_t[77]
    {
        0,0,0,0,0,1,0,0,0,0,0, // .....@.....
        0,0,0,0,1,1,1,0,0,0,0, // ....@@@....
        0,0,0,0,1,1,1,0,0,0,0, // ....@@@....
        0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
    };
    player_sprite = new Sprite(11, 7, player_data);

    // Bullet sprite
    uint8_t* bullet_data = new uint8_t[3] { 1, 1, 1 };
    bullet_sprite = new Sprite(1, 3, bullet_data);

    // Heart (5×5)
    static uint8_t heart_data[25] = {
        0,1,0,1,0,
        1,1,1,1,1,
        1,1,1,1,1,
        0,1,1,1,0,
        0,0,1,0,0
    };
    heartSprite = new Sprite(5, 5, heart_data);

    // Create sprite animations
    alien_animations.clear();
    for (int i = 0; i < 3; ++i) {
        SpriteAnimation* anim = new SpriteAnimation;
        anim->loop = true;
        anim->num_frames = 2;
        anim->frame_duration = 10;
        anim->reset();
        anim->frames = new Sprite*[2];
        anim->frames[0] = alien_sprites[i * 2 + 0];
        anim->frames[1] = alien_sprites[i * 2 + 1];
        alien_animations.push_back(anim);
    }

    delete[] alien_data_A1;
    delete[] alien_data_A2;
    delete[] alien_data_B1;
    delete[] alien_data_B2;
    delete[] alien_data_C1;
    delete[] alien_data_C2;
}


// Shader functions
void validate_shader(GLuint shader, const char* file) {
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char buffer[512];
        glGetShaderInfoLog(shader, sizeof(buffer), nullptr, buffer);
        fprintf(stderr, "Shader compilation failed %s: %s\n", file ? file : "", buffer);
        exit(-1);
    }
}

bool validate_program(GLuint program) {
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        char buffer[512];
        glGetProgramInfoLog(program, sizeof(buffer), nullptr, buffer);
        fprintf(stderr, "Program linking failed: %s\n", buffer);
        return false;
    }
    return true;
}

GLuint create_shader_program() {
    GLuint program = glCreateProgram();
    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vert_shader, 1, &vertex_shader, nullptr);
    glShaderSource(frag_shader, 1, &fragment_shader, nullptr);

    glCompileShader(vert_shader);
    validate_shader(vert_shader, "vertex");

    glCompileShader(frag_shader);
    validate_shader(frag_shader, "fragment");

    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);

    if (!validate_program(program)) {
        glDeleteProgram(program);
        exit(-1);
    }

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    return program;
}

namespace {
    // Game instance pointer for callbacks
    Game* game_instance = nullptr;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    switch(key) {
        case GLFW_KEY_RIGHT:
            if(action == GLFW_PRESS) {
                move_dir += 1;
            }
            else if(action == GLFW_RELEASE) {
                move_dir -= 1;
            }
            break;
        case GLFW_KEY_LEFT:
            if(action == GLFW_PRESS) {
                move_dir -= 1;
            }
            else if(action == GLFW_RELEASE) {
                move_dir += 1;
            }
            break;
        case GLFW_KEY_SPACE:
            if(action == GLFW_RELEASE) {
                fire_pressed = true;
            }
            break;
        case GLFW_KEY_R:
            if(action == GLFW_PRESS && game && game->getState() == GameState::GAME_OVER_PROMPT)
            {
                game->start();
            }
            break;
        default:
            break;
    }
}

void initialize_game_resources() {
    initialize_sprites();
    game->addAlienSprite(AlienType::TYPE_A, alien_sprites[0]);
    game->addAlienSprite(AlienType::TYPE_B, alien_sprites[2]);
    game->addAlienSprite(AlienType::TYPE_C, alien_sprites[4]);
    game->setBulletSprite(bullet_sprite);
    game->setAlienAnimation(AlienType::TYPE_A, alien_animations[0]);
    game->setAlienAnimation(AlienType::TYPE_B, alien_animations[1]);
    game->setAlienAnimation(AlienType::TYPE_C, alien_animations[2]);
}

int main() {
    try{
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        "Space Invaders",
        nullptr,
        nullptr
    );

    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create window");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // enable v-sync
    glfwSetKeyCallback(window, key_callback);
    
    // Initialize GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLEW");
    }

    // Create and use shader program
    std::cout << "Creating shader program..." << std::endl;
    GLuint shader_program = create_shader_program();
    std::cout << "Shader program created!" << std::endl;
    glUseProgram(shader_program);
    glUniform1i(glGetUniformLocation(shader_program, "screenTexture"), 0);

    // Initialize buffer
    const uint32_t colour_background = rgb_to_uint32(0, 128, 0);

    // Create vertex array object for fullscreen quad
    GLuint fullscreen_triangle_vao;
    glGenVertexArrays(1, &fullscreen_triangle_vao);
    glBindVertexArray(fullscreen_triangle_vao);

    // Create texture for the game buffer
    GLuint buffer_texture;
    glGenTextures(1, &buffer_texture);
    glBindTexture(GL_TEXTURE_2D, buffer_texture);

    initialize_sprites(); // ✅ this must happen first
    // Initialize game with buffer dimensions
    game = std::make_unique<Game>(BUFFER_WIDTH, BUFFER_HEIGHT, player_sprite);
    game->setWindow(window);
    initialize_game_resources();  // Set up game resources after game is created

    auto fontMap = createGameFont();
    game->setFont(fontMap);
    game->setHeartSprite(heartSprite);

    // game->start();  // Start the game

    if (!game->getBufferData()) {
        std::cerr << "FATAL: game->getBufferData() is NULL!" << std::endl;
        return -1;
    }
    

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA8,
        BUFFER_WIDTH, BUFFER_HEIGHT, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, game->getBufferData()
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    
    // Disable depth testing since we're doing 2D rendering
    glDisable(GL_DEPTH_TEST);

    game_running = true;
   
    while (!glfwWindowShouldClose(window) && game_running) {
        // Poll inputs
        glfwPollEvents();
    
        // Handle movement input
        if (Player* player = game->getPlayer()) {
            if (move_dir != 0) {
                size_t player_x = player->getX();
                int new_x = static_cast<int>(player_x) + move_dir;
    
                if (new_x + static_cast<int>(player_sprite->getSpriteWidth()) >= static_cast<int>(BUFFER_WIDTH)) {
                    player->setPosition(BUFFER_WIDTH - player_sprite->getSpriteWidth(), player->getY());
                } else if (new_x <= 0) {
                    player->setPosition(0, player->getY());
                } else {
                    player->setPosition(static_cast<size_t>(new_x), player->getY());
                }
            }
    
            if (fire_pressed && player->canShoot()) {
                game->handleInput(0, true);
                fire_pressed = false;
                player->resetShootCooldown();
            }
        }
    
        // Update game logic
        game->run();
    
        // Upload updated buffer to GPU
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0,
            BUFFER_WIDTH, BUFFER_HEIGHT,
            GL_RGBA, GL_UNSIGNED_BYTE,
            game->getBufferData()
        );
    
        // Render fullscreen quad
        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(fullscreen_triangle_vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, buffer_texture);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    
        glfwSwapBuffers(window);
    }
    

    // Cleanup
    for(auto sprite : alien_sprites) {
        delete sprite;
    }
    delete player_sprite;
    delete bullet_sprite;
    for(auto anim : alien_animations) {
        delete[] anim->frames;
        delete anim;
    }
    
    glDeleteTextures(1, &buffer_texture);
    glDeleteProgram(shader_program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;

}catch(const std::exception& e)
    {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
}

// Helper function to clear buffer
void buffer_clear(Buffer* buffer, uint32_t colour) {
    buffer->clear(colour);
}

uint32_t rgb_to_uint32(uint8_t r, uint8_t g, uint8_t b) {
    return (255 << 24) | (g << 16) | (b << 8) | r;  // RGBA format with alpha = 255
}

void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}