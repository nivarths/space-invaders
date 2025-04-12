#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstdint>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <ctime>


// Pre-processor constants
#define GAME_MAX_BULLETS 128

// Structs and enumerations

struct Buffer // struct for the pixel buffer to render the game
{
    size_t width, height;
    uint32_t* data; // represent each pixel as 32 bytes to store four 8-bit colour values for each pixel

};

struct Sprite // struct to draw a character
{
    size_t width, height;
    uint8_t* data;
};

enum AlienType : uint8_t
{
    ALIEN_DEAD = 0,
    ALIEN_TYPE_A = 1,
    ALIEN_TYPE_B = 2,
    ALIEN_TYPE_C = 3
};

struct Alien // struct for the alien with x, y coords and different types of aliens
{
    size_t x, y;
    AlienType type;
};

struct Player // struct for the player with x, y coords and lives for the player
{
    size_t x, y;
    size_t life;
};

struct Bullet // struct for information about the bullet like x, y coords and direction of projectile
{
    size_t x, y;
    int dir;
};

struct Game // struct for the game with the width and height of the area, number of aliens
{
    size_t width, height;
    size_t num_aliens;
    size_t num_bullets;
    size_t num_alien_bullets;
    size_t level;
    Alien* aliens;
    Player player;
    Bullet bullets[GAME_MAX_BULLETS];
    Bullet alien_bullets[GAME_MAX_BULLETS];
};

struct SpriteAnimation // struct to hold information about the sprite animation
{
    bool loop;
    size_t num_frames;
    size_t frame_duration;
    size_t time;
    Sprite** frames;
};

// Global Variables
const size_t buffer_width = 224; // width of the buffer game display
const size_t buffer_height = 256; // height of the buffer game display
Sprite player_sprite;
int player_move_dir = 1; // initializing the player movement to right
bool game_running = false; // global variable to check if game is running
int move_dir = 0; // variable to indicate movement direction
bool fire_pressed = false; // variable to indicate if the projectile was fired
size_t score = 0; // variable to track score
int alien_move_dir = 1; // initializing player movement to right
bool alien_needs_drop = false; // trigger to drop alien when the edge of the board is hit
size_t alien_move_counter = 0; // timer to slow movement
bool game_over = false; // trigger for text that says "GAME OVER" to appear 
bool awaiting_restart = false; // variable to check if the game is waiting for restart prompt to pop up
double last_alien_fire_time;
size_t previous_life = 5;
int heart_fade_timer = 0;
bool just_reset = false;
bool level_transition = false;
double transition_time = 0.0;

Game game;
SpriteAnimation* alien_animations[3]; // index 0 = TYPE_A, 1 = TYPE_B, 2 = TYPE_C
uint8_t* death_counter = nullptr;

// FUNCTION PROTOTYPES
void error_callback(int error, const char* description); // error callback that prints error description
uint32_t rgb_to_uint32(uint8_t r, uint8_t g, uint8_t b); // function to help define colours as uint32_t
void buffer_clear(Buffer* buffer, uint32_t colour); // function to clear the buffer to a certain colour
void validate_shader(GLuint shader, const char* file = 0);
bool validate_program(GLuint program);
void buffer_sprite_draw(Buffer* buffer, const Sprite& sprite, size_t x, size_t y, uint32_t colour); // function to draw sprite pixels at specified coordinates
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
bool sprite_overlap_check(const Sprite& sp_a, size_t x_a, size_t y_a, const Sprite& sp_b, size_t x_b, size_t y_b);
void buffer_draw_text(Buffer* buffer, const Sprite& text_spritesheet, const char* text, size_t x, size_t y, uint32_t colour);
void buffer_draw_number(Buffer* buffer, const Sprite& number_spritesheet, size_t number, size_t x, size_t y, uint32_t colour);
void reset_game(Game& game, uint8_t* death_counter);

// vertex shader
const char* vertex_shader =
    "\n"
    "#version 330\n"
    "\n"
    "noperspective out vec2 TexCoord;\n"
    "\n"
    "void main(void){\n"
    "\n"
    "   TexCoord.x = (gl_VertexID == 2) ? 2.0: 0.0;\n"
    "   TexCoord.y = (gl_VertexID == 1) ? 2.0: 0.0;\n"
    "   \n"
    "   gl_Position = vec4(2.0 * TexCoord - 1.0, 0.0, 1.0);\n"
    "}\n";

// fragment shader
const char* fragment_shader =
    "\n"
    "#version 330\n"
    "\n"
    "uniform sampler2D buffer;\n"
    "noperspective in vec2 TexCoord;\n"
    "\n"
    "out vec3 outColor;\n"
    "\n"
    "void main(void){\n"
    "    outColor = texture(buffer, TexCoord).rgb;\n"
    "}\n";

// MAIN
int main()
{
    // START OF MAIN
    glfwSetErrorCallback(error_callback); // handing over error callback to GLFW to call when error occurs

    // initializing glfw library
    if(!glfwInit())
    {
        return -1;
    }

    // hints to tell GLFW that we would like a context of at least version 3.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // window creation
    GLFWwindow* window = glfwCreateWindow(640, 480, "Space Invaders", NULL, NULL);
    if(!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // enable v-sync
    glfwSetKeyCallback(window, key_callback);

    // initializing GLEW with error handling
    GLenum err = glewInit();
    if(err != GLEW_OK)
    {
        fprintf(stderr, "Error initializing GLEW.\n");
        glfwTerminate();
        return -1;
    }

    // query the OpenGL version asking what version we are getting
    int glVersion[2] = {-1, 1};
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);
    printf("Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);

    // vertex array object (VAO) creation 
    GLuint fullscreen_triangle_vao;
    glGenVertexArrays(1, &fullscreen_triangle_vao);
    glBindVertexArray(fullscreen_triangle_vao);

    // Code to compile the two shaders into code so GPU can understand and link to shader program
    Buffer buffer;
    GLuint shader_id = glCreateProgram();

        // create vertex shader
        {
            GLuint shader_vp = glCreateShader(GL_VERTEX_SHADER);

            glShaderSource(shader_vp, 1, &vertex_shader, nullptr);
            glCompileShader(shader_vp);
            validate_shader(shader_vp, vertex_shader);
            glAttachShader(shader_id, shader_vp);

            glDeleteShader(shader_vp);
        }

        // create fragment shader
        {
            GLuint shader_fp = glCreateShader(GL_FRAGMENT_SHADER);

            glShaderSource(shader_fp, 1, &fragment_shader, 0);
            glCompileShader(shader_fp);
            validate_shader(shader_fp, fragment_shader);
            glAttachShader(shader_id, shader_fp);

            glDeleteShader(shader_fp);
        }

        glLinkProgram(shader_id);

        if(!validate_program(shader_id))
        {
            fprintf(stderr, "Error while validating shader.\n");
            glfwTerminate();
            glDeleteVertexArrays(1, &fullscreen_triangle_vao);
            delete[] buffer.data;
            return -1;
        }

    // code to tell the shader which texture unit to sample from
    glUseProgram(shader_id);
    GLint location = glGetUniformLocation(shader_id, "buffer");
    glUniform1i(location, 0);

    // create and upload buffer
    const uint32_t colour_background = rgb_to_uint32(0, 128, 0);
    // Buffer buffer;
    buffer.width = buffer_width;
    buffer.height = buffer_height;
    buffer.data = new uint32_t[buffer.width * buffer.height];
    buffer_clear(&buffer, colour_background);

    // sprite creation code

    // ALIEN SPRITES
    Sprite alien_sprites[6];
    // alien sprite 0
    alien_sprites[0].width = 8;
    alien_sprites[0].height = 8;
    alien_sprites[0].data = new uint8_t[64]
    {
        0,0,0,1,1,0,0,0, // ...@@...
        0,0,1,1,1,1,0,0, // ..@@@@..
        0,1,1,1,1,1,1,0, // .@@@@@@.
        1,1,0,1,1,0,1,1, // @@.@@.@@
        1,1,1,1,1,1,1,1, // @@@@@@@@
        0,1,0,1,1,0,1,0, // .@.@@.@.
        1,0,0,0,0,0,0,1, // @......@
        0,1,0,0,0,0,1,0  // .@....@.
    };
    // alien sprite 1
    alien_sprites[1].width = 8;
    alien_sprites[1].height = 8;
    alien_sprites[1].data = new uint8_t[64]
    {
        0,0,0,1,1,0,0,0, // ...@@...
        0,0,1,1,1,1,0,0, // ..@@@@..
        0,1,1,1,1,1,1,0, // .@@@@@@.
        1,1,0,1,1,0,1,1, // @@.@@.@@
        1,1,1,1,1,1,1,1, // @@@@@@@@
        0,0,1,0,0,1,0,0, // ..@..@..
        0,1,0,1,1,0,1,0, // .@.@@.@.
        1,0,1,0,0,1,0,1  // @.@..@.@
    };
    // alien sprite 2
    alien_sprites[2].width = 11;
    alien_sprites[2].height = 8;
    alien_sprites[2].data = new uint8_t[88]
    {
        0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
        0,0,0,1,0,0,0,1,0,0,0, // ...@...@...
        0,0,1,1,1,1,1,1,1,0,0, // ..@@@@@@@..
        0,1,1,0,1,1,1,0,1,1,0, // .@@.@@@.@@.
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
        1,0,1,0,0,0,0,0,1,0,1, // @.@.....@.@
        0,0,0,1,1,0,1,1,0,0,0  // ...@@.@@...
    };
    // alien sprite 3
    alien_sprites[3].width = 11;
    alien_sprites[3].height = 8;
    alien_sprites[3].data = new uint8_t[88]
    {
        0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
        1,0,0,1,0,0,0,1,0,0,1, // @..@...@..@
        1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
        1,1,1,0,1,1,1,0,1,1,1, // @@@.@@@.@@@
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
        0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
        0,1,0,0,0,0,0,0,0,1,0  // .@.......@.
    };
    // alien sprite 4
    alien_sprites[4].width = 12;
    alien_sprites[4].height = 8;
    alien_sprites[4].data = new uint8_t[96]
    {
        0,0,0,0,1,1,1,1,0,0,0,0, // ....@@@@....
        0,1,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@@.
        1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
        1,1,1,0,0,1,1,0,0,1,1,1, // @@@..@@..@@@
        1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
        0,0,0,1,1,0,0,1,1,0,0,0, // ...@@..@@...
        0,0,1,1,0,1,1,0,1,1,0,0, // ..@@.@@.@@..
        1,1,0,0,0,0,0,0,0,0,1,1  // @@........@@
    };
    // alien sprite 5
    alien_sprites[5].width = 12;
    alien_sprites[5].height = 8;
    alien_sprites[5].data = new uint8_t[96]
    {
        0,0,0,0,1,1,1,1,0,0,0,0, // ....@@@@....
        0,1,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@@.
        1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
        1,1,1,0,0,1,1,0,0,1,1,1, // @@@..@@..@@@
        1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
        0,0,1,1,1,0,0,1,1,1,0,0, // ..@@@..@@@..
        0,1,1,0,0,1,1,0,0,1,1,0, // .@@..@@..@@.
        0,0,1,1,0,0,0,0,1,1,0,0  // ..@@....@@..
    };
    // alien sprite death
    Sprite alien_death_sprite;
    alien_death_sprite.width = 13;
    alien_death_sprite.height = 7;
    alien_death_sprite.data = new uint8_t[91]
    {
        0,1,0,0,1,0,0,0,1,0,0,1,0, // .@..@...@..@.
        0,0,1,0,0,1,0,1,0,0,1,0,0, // ..@..@.@..@..
        0,0,0,1,0,0,0,0,0,1,0,0,0, // ...@.....@...
        1,1,0,0,0,0,0,0,0,0,0,1,1, // @@.........@@
        0,0,0,1,0,0,0,0,0,1,0,0,0, // ...@.....@...
        0,0,1,0,0,1,0,1,0,0,1,0,0, // ..@..@.@..@..
        0,1,0,0,1,0,0,0,1,0,0,1,0  // .@..@...@..@.
    };
    // player sprite
    player_sprite.width = 11;
    player_sprite.height = 7;
    player_sprite.data = new uint8_t[77]
    {
        0,0,0,0,0,1,0,0,0,0,0, // .....@.....
        0,0,0,0,1,1,1,0,0,0,0, // ....@@@....
        0,0,0,0,1,1,1,0,0,0,0, // ....@@@....
        0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
        1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
    };

    // heart sprite
    Sprite heart_sprite;
    heart_sprite.width = 5;
    heart_sprite.height = 5;
    heart_sprite.data = new uint8_t[25]
    {
        0,1,0,1,0,  // .@.@.
        1,1,1,1,1,  // @@@@@
        1,1,1,1,1,  // @@@@@
        0,1,1,1,0,  // .@@@.
        0,0,1,0,0   // ..@..
    };

    // text spreadsheet sprite
    Sprite text_spritesheet;
    text_spritesheet.width = 5;
    text_spritesheet.height = 7;
    text_spritesheet.data = new uint8_t[65 * 35]
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,
        0,1,0,1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,1,0,1,0,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,0,1,0,1,0,
        0,0,1,0,0,0,1,1,1,0,1,0,1,0,0,0,1,1,1,0,0,0,1,0,1,0,1,1,1,0,0,0,1,0,0,
        1,1,0,1,0,1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,1,1,0,1,0,1,1,
        0,1,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,0,1,0,1,1,1,1,
        0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,
        1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,
        0,0,1,0,0,1,0,1,0,1,0,1,1,1,0,0,0,1,0,0,0,1,1,1,0,1,0,1,0,1,0,0,1,0,0,
        0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
        0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,

        0,1,1,1,0,1,0,0,0,1,1,0,0,1,1,1,0,1,0,1,1,1,0,0,1,1,0,0,0,1,0,1,1,1,0,
        0,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,1,1,0,
        0,1,1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,1,1,0,0,1,0,0,0,1,0,0,0,0,1,1,1,1,1,
        1,1,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
        0,0,0,1,0,0,0,1,1,0,0,1,0,1,0,1,0,0,1,0,1,1,1,1,1,0,0,0,1,0,0,0,0,1,0,
        1,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,1,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
        0,1,1,1,0,1,0,0,0,1,1,0,0,0,0,1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
        1,1,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,
        0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
        0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,1,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0,

        0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,
        0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,
        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,
        0,1,1,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,
        0,1,1,1,0,1,0,0,0,1,1,0,1,0,1,1,1,0,1,1,1,0,1,0,0,1,0,0,0,1,0,1,1,1,0,

        0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,1,
        1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,
        0,1,1,1,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,1,1,1,0,
        1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,
        1,1,1,1,1,1,0,0,0,0,1,0,0,0,0,1,1,1,1,0,1,0,0,0,0,1,0,0,0,0,1,1,1,1,1,
        1,1,1,1,1,1,0,0,0,0,1,0,0,0,0,1,1,1,1,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,
        0,1,1,1,0,1,0,0,0,1,1,0,0,0,0,1,0,1,1,1,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
        1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,
        0,1,1,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,1,1,0,
        0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
        1,0,0,0,1,1,0,0,1,0,1,0,1,0,0,1,1,0,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,0,1,
        1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,1,1,1,1,
        1,0,0,0,1,1,1,0,1,1,1,0,1,0,1,1,0,1,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,
        1,0,0,0,1,1,0,0,0,1,1,1,0,0,1,1,0,1,0,1,1,0,0,1,1,1,0,0,0,1,1,0,0,0,1,
        0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
        1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,
        0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,1,0,1,1,0,0,1,1,0,1,1,1,1,
        1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,1,0,1,0,0,1,0,0,1,0,1,0,0,0,1,
        0,1,1,1,0,1,0,0,0,1,1,0,0,0,0,0,1,1,1,0,1,0,0,0,1,0,0,0,0,1,0,1,1,1,0,
        1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,
        1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
        1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,
        1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,1,0,1,1,0,1,0,1,1,1,0,1,1,1,0,0,0,1,
        1,0,0,0,1,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,1,0,0,0,1,
        1,0,0,0,1,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,
        1,1,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,1,1,1,1,

        0,0,0,1,1,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,1,
        0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,
        1,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,1,1,0,0,0,
        0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
        0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };

    // number sprite
    Sprite number_spritesheet = text_spritesheet;
    number_spritesheet.data += 16 * 35;

    // bullet sprite
    Sprite bullet_sprite;
    bullet_sprite.width = 1;
    bullet_sprite.height = 3;
    bullet_sprite.data = new uint8_t[3]
    {
        1, // @
        1, // @
        1  // @
    };

    // animation for the alien sprites

    for(int i = 0; i < 3; ++i)
    {
        alien_animations[i] = new SpriteAnimation;
        alien_animations[i]->loop = true;
        alien_animations[i]->num_frames = 2;
        alien_animations[i]->frame_duration = 10;
        alien_animations[i]->time = 0;
        alien_animations[i]->frames = new Sprite*[2];
        alien_animations[i]->frames[0] = &alien_sprites[i * 2 + 0]; // 0, 2, 4
        alien_animations[i]->frames[1] = &alien_sprites[i * 2 + 1]; // 1, 3, 5
    }
    

    // initializing game struct
    game.width = buffer_width;
    game.height = buffer_height;
    game.num_aliens = 55;
    game.aliens = new Alien[game.num_aliens];

    death_counter = new uint8_t[game.num_aliens];
    for(size_t i = 0; i < game.num_aliens; ++i)
    {
        death_counter[i] = 10;
    }

    const size_t player_start_x = buffer_width / 2 - player_sprite.width / 2;
    game.player.x = player_start_x;
    game.player.y = 32;
    game.player.life = 5;
    previous_life = game.player.life;
    game.level = 1;
    game.num_bullets = 0;
    game.num_alien_bullets = 0;

    // code to initialize alien positions
    for(size_t yi = 0; yi < 5; ++yi)
    {
        for(size_t xi = 0; xi < 11; ++xi)
        {
            size_t index = yi * 11 + xi;
            game.aliens[yi * 11 + xi].x = 16 * xi + 20;
            game.aliens[yi * 11 + xi].y = 17 * yi + 128;

            if(yi == 0)
            {
                game.aliens[index].type = ALIEN_TYPE_C;
            }
            else if(yi < 3)
            {
                game.aliens[index].type = ALIEN_TYPE_B;
            }
            else
            {
                game.aliens[index].type = ALIEN_TYPE_A;
            }
        }
    }

    // create buffer texture
    GLuint buffer_texture;
    glGenTextures(1, &buffer_texture);

    glBindTexture(GL_TEXTURE_2D, buffer_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, buffer.width, buffer.height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.data);
    for(size_t bi = 0; bi < game.num_bullets; ++bi)
    {
        const Bullet& bullet = game.bullets[bi];
        const Sprite& sprite = bullet_sprite;
        buffer_sprite_draw(&buffer, sprite, bullet.x, bullet.y, rgb_to_uint32(128, 0, 0));
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // code to disable depth testing and bind vertex array previously made
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(fullscreen_triangle_vao);

    // GAME LOOP

    glClearColor(1.0, 0.0, 0.0, 1.0);
    game_running = true;
    while(!glfwWindowShouldClose(window) && game_running)
    {
        double now = glfwGetTime();

        if(level_transition)
        {
            double now = glfwGetTime();
            double elapsed = now - transition_time;
            
            buffer_clear(&buffer, colour_background);

            const char* transition_message = "LEVEL";
            size_t transition_message_x = (buffer.width - 5 * (text_spritesheet.width + 1)) / 2;
            size_t transition_message_y = buffer.height / 2 + text_spritesheet.height;
            buffer_draw_text(&buffer, text_spritesheet, transition_message, transition_message_x, transition_message_y, rgb_to_uint32(255, 255, 255));
            
            char level_text[8];
            snprintf(level_text, sizeof(level_text), "%zu", game.level + 1);
            size_t num_x = (buffer.width - number_spritesheet.width) / 2;
            size_t num_y = transition_message_y - 2 * text_spritesheet.height;
            buffer_draw_number(&buffer, number_spritesheet, game.level + 1, num_x, num_y, rgb_to_uint32(255, 255, 255));

            if(elapsed >= 3.0)
            {
                ++game.level;
                alien_move_counter = 0;
                last_alien_fire_time = glfwGetTime();

                for(int anim_i = 0; anim_i < 3; ++anim_i)
                {
                    alien_animations[anim_i]->time = 0;
                }

                for(size_t i = 0; i < game.num_aliens; ++i)
                {
                    death_counter[i] = 10;

                    size_t xi = i % 11;
                    size_t yi = i / 11;
                    game.aliens[i].x = 16 * xi + 20;
                    game.aliens[i].y = 17 * yi + 128;

                    if(yi == 0)
                    {
                        game.aliens[i].type = ALIEN_TYPE_C;
                    }
                    else if(yi < 3)
                    {
                        game.aliens[i].type = ALIEN_TYPE_B;
                    }
                    else
                    {
                        game.aliens[i].type = ALIEN_TYPE_A;
                    }
                }

                level_transition = false;
            }

            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, buffer.width, buffer.height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.data);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }
        
        if(game.player.life < previous_life)
        {
            heart_fade_timer = 30;
        }
        previous_life = game.player.life;

        glClear(GL_COLOR_BUFFER_BIT);
        buffer_clear(&buffer, colour_background); // clear buffer each frame

        // UI DRAWING //
        buffer_draw_text(&buffer, text_spritesheet, "SCORE", 4, game.height - text_spritesheet.height - 7, rgb_to_uint32(128, 0, 0));
        buffer_draw_number(&buffer, number_spritesheet, score, 4 + 2 * number_spritesheet.width, game.height - 2 * number_spritesheet.height - 12, rgb_to_uint32(128, 0, 0));

        // Drawing lives and hearts
        const char* lives_label = "LIVES";
        const size_t lives_label_len = 5;
        const size_t char_width = text_spritesheet.width;
        const size_t char_spacing = 1;
        const size_t label_width = lives_label_len * (char_width + char_spacing) - char_spacing;
        const size_t lives_label_x = (buffer.width - label_width) / 2;
        const size_t label_y = game.height - text_spritesheet.height - 7;
        buffer_draw_text(&buffer, text_spritesheet, lives_label, lives_label_x, label_y, rgb_to_uint32(128, 0, 0));
        
        const size_t heart_spacing = 2;
        size_t total_heart_width = game.player.life * heart_sprite.width + (game.player.life - 1) * heart_spacing;
        size_t heart_start_x = (buffer.width - total_heart_width) / 2;
        size_t heart_y = label_y - heart_sprite.height - 2;
        for(size_t i = 0; i < game.player.life; ++i)
        {
            size_t x = heart_start_x + i * (heart_sprite.width + heart_spacing);
            buffer_sprite_draw(&buffer, heart_sprite, x, heart_y, rgb_to_uint32(255, 0, 0));
        }

        // drawing level and level number
        const char* level_label = "LEVEL";
        const size_t level_x = buffer.width - 5 * (text_spritesheet.width + 1) - 20;
        const size_t level_y = game.height - text_spritesheet.height - 7;
        buffer_draw_text(&buffer, text_spritesheet, level_label, level_x, level_y, rgb_to_uint32(128, 0, 0));

        size_t level_number_x = level_x + 5 * (text_spritesheet.width + 1) + 4;
        buffer_draw_number(&buffer, number_spritesheet, game.level, level_number_x, level_y, rgb_to_uint32(128, 0, 0));

        if(heart_fade_timer > 0 && game.player.life < 5)
        {
            if((heart_fade_timer / 5) % 2 == 0)
            {
                const size_t ghost_x = heart_start_x + game.player.life * (heart_sprite.width + heart_spacing);
                buffer_sprite_draw(&buffer, heart_sprite, ghost_x, heart_y, rgb_to_uint32(100, 100, 100));
            }

            --heart_fade_timer;
        }

        buffer_draw_text(&buffer, text_spritesheet, "CREDIT 00", 164, 7, rgb_to_uint32(128, 0, 0));
        for(size_t i = 0; i < game.width; ++i)
        {
            buffer.data[game.width * 16 + i] = rgb_to_uint32(128, 0, 0);
        }

        if(!game_over)
        {
            // alien movement logic
            alien_move_counter++;
            int raw_speed = 15 - static_cast<int>(game.level * 2);
            size_t alien_speed = static_cast<size_t>(std::max(3, raw_speed));

            if(alien_move_counter >= alien_speed)
            {
                alien_move_counter = 0;

                // edge collision check
                bool edge_hit = false;
                for(size_t ai = 0; ai < game.num_aliens; ++ai)
                {
                    const Alien& alien = game.aliens[ai];
                    if(alien.type == ALIEN_DEAD)
                    {
                        continue;
                    }

                    size_t alien_right = alien.x + 12; // approximate alien width
                    if((alien_move_dir == 1 && alien_right > game.width - 10) || (alien_move_dir == -1 && alien.x < 10))
                    {
                        edge_hit = true;
                        break;
                    }
                }

                if(edge_hit)
                {
                    alien_move_dir *= -1;
                    alien_needs_drop = true;
                }

                for(size_t ai = 0; ai < game.num_aliens; ++ai)
                {
                    Alien& alien = game.aliens[ai];
                    if(alien.type == ALIEN_DEAD)
                    {
                        continue;
                    }

                    if(alien_needs_drop)
                    {
                        alien.y -= 8;
                    }
                    else
                    {
                        alien.x += alien_move_dir * 4;
                    }
                }

                for(size_t ai = 0; ai < game.num_aliens; ++ai)
                {
                    const Alien& alien = game.aliens[ai];
                    if(alien.type != ALIEN_DEAD)
                    {
                        if(alien.y <= game.player.y + player_sprite.height)
                        {
                            game_over = true;
                            double freeze_start_time = glfwGetTime();
                            while(glfwGetTime() - freeze_start_time < 5.0)
                            {
                                buffer_clear(&buffer, colour_background);
                                
                                buffer_draw_text(&buffer, text_spritesheet, "SCORE", 4, game.height - text_spritesheet.height - 7, rgb_to_uint32(128, 0, 0));
                                buffer_draw_number(&buffer, number_spritesheet, score, 4 + 2 * number_spritesheet.width, game.height - 2 * number_spritesheet.height - 12, rgb_to_uint32(128, 0, 0));
                                buffer_draw_text(&buffer, text_spritesheet, "CREDIT 00", 164, 7, rgb_to_uint32(128, 0, 0));
                                
                                for(size_t i = 0; i < game.width; ++i)
                                {
                                    buffer.data[game.width * 16 + i] = rgb_to_uint32(128, 0, 0);
                                }
                                
                                const char* message = "GAME OVER";
                                size_t message_width = 9 * (text_spritesheet.width + 1);
                                size_t x = (buffer.width - message_width) / 2;
                                size_t y = buffer.height / 2 - text_spritesheet.height / 2;
                                buffer_draw_text(&buffer, text_spritesheet, message, x, y, rgb_to_uint32(255, 255, 255));

                                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, buffer.width, buffer.height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.data);
                                glDrawArrays(GL_TRIANGLES, 0, 3);
                                glfwSwapBuffers(window);
                                glfwPollEvents();
                            }

                            break;
                        }
                    }
                    
                }

                alien_needs_drop = false;
            }

            // alien bullet firing logic
            srand(static_cast<unsigned int>(std::time(NULL)));
            double current_time = glfwGetTime();
            double alien_fire_interval = std::max(0.2, 1.0 - game.level * 0.1);
            if(current_time - last_alien_fire_time >= alien_fire_interval && game.num_alien_bullets < GAME_MAX_BULLETS)
            {
                std::vector<size_t> alive_indices;
                for(size_t i = 0; i < game.num_aliens; ++i)
                {
                    if(game.aliens[i].type != ALIEN_DEAD)
                    {
                        alive_indices.push_back(i);
                    }
                }

                if(!alive_indices.empty())
                {
                    size_t random_index = alive_indices[rand() % alive_indices.size()];
                    const Alien& shooter = game.aliens[random_index];

                    game.alien_bullets[game.num_alien_bullets].x = shooter.x + 4;
                    game.alien_bullets[game.num_alien_bullets].y = shooter.y;
                    game.alien_bullets[game.num_alien_bullets].dir = -2;
                    ++game.num_alien_bullets;

                    last_alien_fire_time = current_time;
                }
            }
        }

        if(!game_over || !awaiting_restart)
        {
            // Draw all aliens
            for(size_t ai = 0; ai < game.num_aliens; ++ai)
            {
                if(!death_counter[ai])
                {
                    continue;
                }

                const Alien& alien = game.aliens[ai];

                if(alien.type == ALIEN_DEAD)
                {
                    buffer_sprite_draw(&buffer, alien_death_sprite, alien.x, alien.y, rgb_to_uint32(128, 0, 0));
                }
                else
                {
                    const SpriteAnimation& animation = *alien_animations[alien.type - 1];
                    size_t current_frame = animation.time / animation.frame_duration;
                    const Sprite& sprite = *animation.frames[current_frame];
                    buffer_sprite_draw(&buffer, sprite, alien.x, alien.y, rgb_to_uint32(128, 0, 0));
                }
            }

            for(size_t ai = 0; ai < game.num_aliens; ++ai)
            {
                const Alien& alien = game.aliens[ai];
                if(alien.type == ALIEN_DEAD && death_counter[ai])
                {
                    --death_counter[ai];
                }
            }

            buffer_sprite_draw(&buffer, player_sprite, game.player.x, game.player.y, rgb_to_uint32(128, 0, 0));

            // draw bullets
            for(size_t bi = 0; bi < game.num_bullets; ++bi)
            {
                const Bullet& bullet = game.bullets[bi];
                const Sprite& sprite = bullet_sprite;
                buffer_sprite_draw(&buffer, sprite, bullet.x, bullet.y, rgb_to_uint32(128, 0, 0));
            }

            // draw alien bullets
            for(size_t bi = 0; bi < game.num_alien_bullets; ++bi)
            {
                const Bullet& bullet = game.alien_bullets[bi];
                const Sprite& sprite = bullet_sprite;
                buffer_sprite_draw(&buffer, sprite, bullet.x, bullet.y, rgb_to_uint32(255, 255, 255));
            }

            // bullet movement
            for(size_t bi = 0; bi < game.num_bullets;)
            {
                game.bullets[bi].y += game.bullets[bi].dir;

                // out of bounds check
                if(game.bullets[bi].y >= game.height || game.bullets[bi].y < bullet_sprite.height)
                {
                    game.bullets[bi] = game.bullets[game.num_bullets - 1];
                    --game.num_bullets;
                    continue;
                }

                // checking collision with aliens
                bool hit = false;
                for(size_t ai = 0; ai < game.num_aliens; ++ai)
                {
                    const Alien& alien = game.aliens[ai];
                    if(alien.type == ALIEN_DEAD)
                    {
                        continue;
                    }

                    const SpriteAnimation& animation = *alien_animations[alien.type - 1];
                    size_t current_frame = animation.time / animation.frame_duration;
                    const Sprite& alien_sprite = *animation.frames[current_frame];
                    bool overlap = sprite_overlap_check(bullet_sprite, game.bullets[bi].x, game.bullets[bi].y, alien_sprite, alien.x, alien.y);

                    if(overlap)
                    {
                        score += 10 * (4 - game.aliens[ai].type);
                        game.aliens[ai].type = ALIEN_DEAD;
                        game.aliens[ai].x -= (alien_death_sprite.width - alien_sprite.width) / 2;
                        game.bullets[bi] = game.bullets[game.num_bullets - 1];
                        --game.num_bullets;
                        hit = true;
                        continue;
                    }
                }

                if(!hit)
                {
                    ++bi;
                }
            }

            // alien bullet movement (with player collision)
            for(size_t bi = 0; bi < game.num_alien_bullets;)
            {
                game.alien_bullets[bi].y += game.alien_bullets[bi].dir;

                // out of bounds check
                if(game.alien_bullets[bi].y < bullet_sprite.height)
                {
                    game.alien_bullets[bi] = game.alien_bullets[game.num_alien_bullets - 1];
                    --game.num_alien_bullets;
                    continue;
                }

                // player collision
                if(sprite_overlap_check(bullet_sprite, game.alien_bullets[bi].x, game.alien_bullets[bi].y, player_sprite, game.player.x, game.player.y))
                {
                    if(game.player.life > 0)
                    {
                        --game.player.life;
                    }

                    if(game.player.life == 0)
                    {
                        game_over = true;
                    }

                    game.alien_bullets[bi] = game.alien_bullets[game.num_alien_bullets - 1];
                    --game.num_alien_bullets;
                    continue;
                }

                ++bi;
            }

            // check if all aliens are dead
            bool all_aliens_dead = true;
            for(size_t ai = 0; ai < game.num_aliens; ++ai)
            {
                if(game.aliens[ai].type != ALIEN_DEAD)
                {
                    all_aliens_dead = false;
                    break;
                }
            }

            if(all_aliens_dead && !just_reset && !level_transition)
            {
                level_transition = true;
                transition_time = glfwGetTime();
            }
        }
        // game over conditions
        if(game_over)
        {
            const char* message = "GAME OVER";
            size_t message_width = 9 * (text_spritesheet.width + 1);
            size_t x = (buffer.width - message_width) / 2;
            size_t y = buffer.height / 2 - text_spritesheet.height / 2;
            buffer_draw_text(&buffer, text_spritesheet, message, x, y, rgb_to_uint32(255, 255, 255));

            const char* restart_prompt = "PRESS R TO RESTART";
            size_t restart_prompt_width = 18 * (text_spritesheet.width + 1);
            size_t px = (buffer.width - restart_prompt_width) / 2;
            size_t py = y - 2 * text_spritesheet.height;
            buffer_draw_text(&buffer, text_spritesheet, restart_prompt, px, py, rgb_to_uint32(255, 255, 255));
            awaiting_restart = true;
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, buffer.width, buffer.height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer.data);
        
        // advance the alien animation
        for(int i = 0; i < 3; ++i)
        {
            ++alien_animations[i]->time;
            if(alien_animations[i]->time == alien_animations[i]->num_frames * alien_animations[i]->frame_duration)
            {
                if(alien_animations[i]->loop)
                {
                    alien_animations[i]->time = 0;
                }
                else
                {
                    delete alien_animations[i];
                    alien_animations[i] = nullptr;
                }
            }
        }
        
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // player movement
        int player_move_dir = 2* move_dir;

        if(player_move_dir != 0)
        {
            if(game.player.x + player_sprite.width + player_move_dir >= game.width)
            {
                game.player.x = game.width - player_sprite.width;
            }
            else if((int)game.player.x + player_move_dir <= 0)
            {
                game.player.x = 0;
            }
            else
            {
                game.player.x += player_move_dir;
            }
        }

        // bullet movement
        if(fire_pressed && game.num_bullets < GAME_MAX_BULLETS)
        {
            game.bullets[game.num_bullets].x = game.player.x + player_sprite.width / 2;
            game.bullets[game.num_bullets].y = game.player.y + player_sprite.height;
            game.bullets[game.num_bullets].dir = 2;
            ++game.num_bullets;
        }
        fire_pressed = false;
        // reset flag so level can increment from 1 when game restarts
        just_reset = false;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // memory cleanup
    delete[] buffer.data;
    for(int i = 0; i < 6; ++i)
    {
        delete[] alien_sprites[i].data;
    }
    delete[] heart_sprite.data;
    delete[] game.aliens;
    delete[] player_sprite.data;
    for(int i = 0; i < 3; ++i)
    {
        delete[] alien_animations[i]->frames;
        delete alien_animations[i];
    }
    glfwDestroyWindow(window);

    // END OF MAIN
}


// FUNCTION DEFINITIONS

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
    
}

uint32_t rgb_to_uint32(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 24) | (g << 16) | (b << 8) | 255;
}

void buffer_clear(Buffer* buffer, uint32_t colour)
{
    for(size_t i = 0; i < buffer->width * buffer->height; ++i)
    {
        buffer->data[i] = colour;
    }
}

void validate_shader(GLuint shader, const char* file)
{
    static const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    GLsizei length = 0;

    glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);

    if(length > 0)
    {
        printf("Shader %d(%s) compile error: %s\n", shader, (file ? file: ""), buffer);
    }
}

bool validate_program(GLuint program)
{
    static const GLsizei BUFFER_SIZE = 512;
    GLchar buffer[BUFFER_SIZE];
    GLsizei length = 0;

    glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);

    if(length > 0)
    {
        printf("Program %d link error: %s\n", program, buffer);
        return false;
    }

    return true;
}

void buffer_sprite_draw(Buffer* buffer, const Sprite& sprite, size_t x, size_t y, uint32_t colour)
{
    for(size_t xi = 0; xi < sprite.width; ++xi)
    {
        for(size_t yi = 0; yi < sprite.height; ++yi)
        {
            size_t sy = sprite.height - 1 + y - yi;
            size_t sx = x + xi;
            if(sprite.data[yi * sprite.width + xi] && sy < buffer -> height && sx < buffer->width)
            {
                buffer->data[sy * buffer->width + sx] = colour;
            }
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch(key)
    {
        case GLFW_KEY_ESCAPE:
            if(action == GLFW_PRESS)
            {
                game_running = false;
            }
            break;
        case GLFW_KEY_RIGHT:
            if(action == GLFW_PRESS)
            {
                move_dir += 1;
            }
            else if(action == GLFW_RELEASE)
            {
                move_dir -= 1;
            }
            break;
        case GLFW_KEY_LEFT:
            if(action == GLFW_PRESS)
            {
                move_dir -= 1;
            }
            else if(action == GLFW_RELEASE)
            {
                move_dir += 1;
            }
            break;
        case GLFW_KEY_SPACE:
            if(action == GLFW_RELEASE)
            {
                fire_pressed = true;
            }
            break;
        case GLFW_KEY_R:
            if(action == GLFW_PRESS && game_over)
            {
                awaiting_restart = false;
                reset_game(game, death_counter);
            }
            break;
        default:
            break;
    }
}

bool sprite_overlap_check(const Sprite& sp_a, size_t x_a, size_t y_a, const Sprite& sp_b, size_t x_b, size_t y_b)
{
    if(x_a < x_b + sp_b.width && x_a + sp_a.width > x_b && y_a < y_b + sp_b.height && y_a + sp_a.height > y_b)
    {
        return true;
    }

    return false;
}

void buffer_draw_text(Buffer* buffer, const Sprite& text_spritesheet, const char* text, size_t x, size_t y, uint32_t colour)
{
    size_t xp = x;
    size_t stride = text_spritesheet.width * text_spritesheet.height;
    Sprite sprite = text_spritesheet;
    for(const char* charp = text; *charp != '\0'; ++charp)
    {
        char character = *charp - 32;
        if(character < 0 || character >= 65)
        {
            continue;
        }

        sprite.data = text_spritesheet.data + character * stride;
        buffer_sprite_draw(buffer, sprite, xp, y, colour);
        xp += sprite.width + 1;
    }
}

void buffer_draw_number(Buffer* buffer, const Sprite& number_spritesheet, size_t number, size_t x, size_t y, uint32_t colour)
{
    uint8_t digits[64];
    size_t num_digits = 0;
    size_t current_number = number;

    do
    {
        digits[num_digits++] = current_number % 10;
        current_number = current_number / 10;
    } while (current_number > 0);

    size_t xp = x;
    size_t stride = number_spritesheet.width * number_spritesheet.height;
    Sprite sprite = number_spritesheet;
    
    for(size_t i = 0; i < num_digits; ++i)
    {
        uint8_t digit = digits[num_digits - i - 1];
        sprite.data = number_spritesheet.data + digit * stride;
        buffer_sprite_draw(buffer, sprite, xp, y, colour);
        xp += sprite.width + 1;
    }
}

void reset_game(Game& game, uint8_t* death_counter)
{
    score = 0;
    game_over = false;
    move_dir = 0;
    fire_pressed = false;
    alien_move_dir = 1;
    alien_needs_drop = false;
    alien_move_counter = 0;
    awaiting_restart = false;
    game_running = true;
    last_alien_fire_time = glfwGetTime();
    game.num_bullets = 0;
    game.num_alien_bullets = 0;
    const size_t player_start_x = buffer_width / 2 - player_sprite.width / 2;
    game.player.x = player_start_x;
    game.player.y = 32;
    game.player.life = 5;
    game.level = 1;
    just_reset = true;

    for(size_t i = 0; i < game.num_aliens; ++i)
    {
        death_counter[i] = 10;

        size_t xi = i % 11;
        size_t yi = i / 11;
        game.aliens[i].x = 16 * xi + 20;
        game.aliens[i].y = 17 * yi + 128;

        if(yi == 0)
        {
            game.aliens[i].type = ALIEN_TYPE_C;
        }
        else if(yi < 3)
        {
            game.aliens[i].type = ALIEN_TYPE_B;
        }
        else
        {
            game.aliens[i].type = ALIEN_TYPE_A;
        }

    }

    for(int i = 0; i < 3; ++i)
    {
        alien_animations[i]->time = 0;
    }
}