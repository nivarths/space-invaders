#pragma once

// Basic shader code
const char* vertex_shader = R"(
    #version 330 core

    const vec2 vertices[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2(3.0, -1.0),
        vec2(-1.0, 3.0)
    );

    const vec2 texCoords[3] = vec2[](
        vec2(0.0, 0.0),
        vec2(2.0, 0.0),
        vec2(0.0, 2.0)
    );

    out vec2 texCoord;

    void main() {
        gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
        texCoord = texCoords[gl_VertexID];
    }
)";


    
const char* fragment_shader = R"(
    #version 330 core
    in vec2 texCoord;
    out vec4 FragColor;
    
    uniform sampler2D screenTexture;
    
    void main() {
        FragColor = texture(screenTexture, texCoord);
    }
)";
    
    