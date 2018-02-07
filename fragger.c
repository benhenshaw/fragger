//
// FRAGGER
// A super simple fragment shader playground.
// by Benedict Henshaw
// 2018-02-06
//

#include <SDL2/SDL.h>
#include <SDL2/SDL_stdinc.h>
#include "glad.c"

void panic_exit() {
    puts(SDL_GetError());
    exit(1);
}

int main(int argument_count, char ** arguments) {
    setbuf(stdout, NULL);

    int error = SDL_Init(SDL_INIT_VIDEO);
    if (error) exit(__LINE__);

    SDL_GL_LoadLibrary(NULL);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetSwapInterval(1);

    SDL_Window * window = SDL_CreateWindow("",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480,
        /*SDL_WINDOW_FULLSCREEN_DESKTOP |*/
        SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_OPENGL |
        SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) exit(__LINE__);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) exit(__LINE__);

    gladLoadGLLoader(SDL_GL_GetProcAddress);

    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    int scale = 2;
    width  *= scale;
    height *= scale;
    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    // printf("Renderer: %s\n", glGetString(GL_RENDERER));
    // printf("Version:  %s\n", glGetString(GL_VERSION));
    // printf("\n");

    char vert[] = "#version 330\n"
                  "in vec4 vert;\n"
                  "void main() {\n"
                  "    gl_Position = vert;\n"
                  "}\n";

    char * frag_file_name = "frag.glsl";
    if (argument_count > 1) frag_file_name = arguments[1];

    char * frag = calloc(4096, 1);
    FILE * frag_file = fopen(frag_file_name, "r");
    if (!frag_file) {
        printf("ERROR: Could not open '%s'.\n", frag_file_name);
        return 1;
    }
    if (!fread(frag, 1, 4096, frag_file)) exit(__LINE__);

    int status = 0;

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const char * []) { vert }, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char message[512];
        glGetShaderInfoLog(vertex_shader, 512, NULL, message);
        puts(message);
        printf("ERROR: Vertex shader compilation failed.\n");
        return 1;
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const char * []) { frag }, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char message[512];
        glGetShaderInfoLog(fragment_shader, 512, NULL, message);
        puts(message);
        printf("ERROR: Fragment shader compilation failed.\n");
        return 1;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char message[512];
        glGetShaderInfoLog(fragment_shader, 512, NULL, message);
        puts(message);
        printf("ERROR: Shader program link failed.\n");
        return 1;
    }

    glUseProgram(program);

    GLuint vertex_array_object[1];
    GLuint buffers[1];
    const GLuint vertex_count = 6;

    glGenVertexArrays(1, vertex_array_object);
    glBindVertexArray(vertex_array_object[0]);

    GLfloat vertices[vertex_count][2] = {
        { -1.0, -1.0 },
        {  1.0, -1.0 },
        {  1.0,  1.0 },
        {  1.0,  1.0 },
        { -1.0,  1.0 },
        { -1.0, -1.0 }
    };

    glGenBuffers(1, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glUniform2f(glGetUniformLocation(program, "resolution"), width, height);

    while (1) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            } else if (event.type == SDL_MOUSEMOTION) {
                glUniform2f(glGetUniformLocation(program, "mouse"),
                    event.motion.x, height - event.motion.y);
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    width = event.window.data1 * scale;
                    height = event.window.data2 * scale;
                    glUniform2f(glGetUniformLocation(program, "resolution"), width, height);
                }
            }
        }

        glUniform1f(glGetUniformLocation(program, "time"), SDL_GetTicks() / 1000.0f);

        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(vertex_array_object[0]);
        glDrawArrays(GL_TRIANGLES, 0, vertex_count);
        SDL_GL_SwapWindow(window);
    }
}
