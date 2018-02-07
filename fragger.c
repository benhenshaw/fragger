//
// FRAGGER
// A super simple fragment shader playground.
// by Benedict Henshaw
// 2018-02-06
//

#include <SDL2/SDL.h>
#include "glad.c"

typedef Uint64 u64;

// You can change the names of the uniforms here if you like.
#define UNIFORM_RESOLUTION "resolution"
#define UNIFORM_MOUSE "mouse"
#define UNIFORM_TIME "time"
#define UNIFORM_RANDOM "random"
#define UNIFORM_BUTTON "button"

// Exit the program, displaying an error message via pop-up box and print out.
void panic_exit(char * message, ...) {
    va_list args;
    va_start(args, message);
    char buffer[512];
    vsnprintf(buffer, 512, message, args);
    va_end(args);
    puts(buffer);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", buffer, NULL);
    exit(1);
}

// Pseudo-random number generator.
// (Xoroshiro128+)
u64 random_seed[2] = { 0, 0 };

u64 random_u64() {
    u64 s0 = random_seed[0];
    u64 s1 = random_seed[1];
    u64 result = s0 + s1;
    s1 ^= s0;
    #define LS(x, k) ((x << k) | (x >> (64 - k)))
    random_seed[0] = LS(s0, 55) ^ s1 ^ (s1 << 14);
    random_seed[1] = LS(s1, 36);
    #undef LS
    return result;
}

// Set the seed for the pseudo-random number generator.
void set_seed(u64 a, u64 b) {
    random_seed[0] = a;
    random_seed[1] = b;
    for (int i = 0; i < 64; ++i) random_u64();
}

// Get a random float between 0.0 and 1.0.
float random_float() {
    return (float)random_u64() / (float)UINT64_MAX;
}

int main(int argument_count, char ** arguments) {
    // Disable output buffering.
    // (This helps for some text editors, such as Sublime Text.)
    setbuf(stdout, NULL);

    // Handle program arguments.
    int retina_mode = 0;
    int debug_mode = 0;
    char * frag_file_name = NULL;

    if (argument_count > 1) {
        for (int i = 1; i < argument_count; ++i) {
            if (arguments[i][0] == '-') {
                if (arguments[i][1] == 'r') retina_mode = 1; else
                if (arguments[i][1] == 'd') debug_mode = 1;
            } else {
                if (!frag_file_name) frag_file_name = arguments[i];
            }
        }
    }

    // If no file was given, fall back to the default.
    if (!frag_file_name) frag_file_name = "frag.glsl";

    // Print some debug info.
    if (debug_mode) {
        printf(
            "FRAGGER (Debug)\n"
            "File: %s\n"
            "Retina Mode: %s\n",
            frag_file_name,
            retina_mode ? "true" : "false"
        );
    }

    // Attempt to initialise SDL2.
    int error = SDL_Init(SDL_INIT_VIDEO);
    if (error) {
        panic_exit("Could not initialise SDL2.\n%s", SDL_GetError());
    }

    // Set some flags for the window.
    int window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;
    // Only allow high-dpi if the retina flag is set.
    if (retina_mode) window_flags |= SDL_WINDOW_ALLOW_HIGHDPI;

    SDL_Window * window = SDL_CreateWindow("",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480,
        window_flags);
    if (!window) {
        panic_exit("Could not create window.\n%s", SDL_GetError());
    }

    // Set up OpenGL context.
    SDL_GL_LoadLibrary(NULL);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetSwapInterval(1);

    // Attempt to create the context.
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        panic_exit("Could not create OpenGL context.\n%s", SDL_GetError());
    }

    // Dynamically load the OpenGL functions.
    gladLoadGLLoader(SDL_GL_GetProcAddress);

    // Get the window dimensions.
    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    // Assume 2x scale when in retina mode.
    // (This is not very robust, but is better than nothing.)
    int scale = retina_mode ? 2 : 1;

    // Our window dimensions do not take into account retina scale, so scale them.
    width  *= scale;
    height *= scale;

    glViewport(0, 0, width, height);

    // We do not need these features, so disable them.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Clear to black.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Print some GL context info if in debug mode.
    if (debug_mode) {
        printf("Vendor:   %s\n", glGetString(GL_VENDOR));
        printf("Renderer: %s\n", glGetString(GL_RENDERER));
        printf("Version:  %s\n", glGetString(GL_VERSION));
        printf("\n");
    }

    // Read in the fragment shader file.
    // Attempt to open the file.
    FILE * frag_file = fopen(frag_file_name, "r");
    if (!frag_file) {
        panic_exit("Could not open file '%s'.", frag_file_name);
    }
    // Get the length of the file.
    fseek(frag_file, 0, SEEK_END);
    int file_length = ftell(frag_file);
    rewind(frag_file);
    // Allocate enough space for the whole file.
    char * frag = calloc(file_length + 1, 1);
    if (!frag) panic_exit("Could not allocate space for the fragment source.");
    // Attempt to read the whole file.
    if (!fread(frag, 1, file_length, frag_file)) {
        panic_exit("Could not read from file '%s'.", frag_file_name);
    }

    // This will hold the status of the shader compilation.
    int status = 0;

    // Use the simplest possible vertex shader.
    char vert[] = "#version 330\n"
                  "in vec4 vert;\n"
                  "void main() {\n"
                  "    gl_Position = vert;\n"
                  "}\n";
    // Attempt to compile the vertex shader.
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const char * []) { vert }, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char message[512];
        glGetShaderInfoLog(vertex_shader, 512, NULL, message);
        panic_exit("Vertex shader compilation failed:\n%s", message);
    }

    // Attempt to compile the fragment shader.
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const char * []) { frag }, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char message[512];
        glGetShaderInfoLog(fragment_shader, 512, NULL, message);
        panic_exit("Fragment shader ('%s') compilation failed:\n%s", frag_file_name, message);
    }

    // Attempt to create and link the program.
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char message[512];
        glGetShaderInfoLog(fragment_shader, 512, NULL, message);
        panic_exit("Shader program link failed.");
    }

    // Make this the active program.
    glUseProgram(program);

    // Load two triangles that will cover the whole screen.
    GLuint vertex_array_object[1];
    GLuint buffers[1];
    const GLuint vertex_count = 6;
    glGenVertexArrays(1, vertex_array_object);
    glBindVertexArray(vertex_array_object[0]);
    GLfloat vertices[vertex_count][2] = {
        { -1.0, -1.0 }, {  1.0, -1.0 }, {  1.0,  1.0 },
        {  1.0,  1.0 }, { -1.0,  1.0 }, { -1.0, -1.0 }
    };
    glGenBuffers(1, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    set_seed(SDL_GetPerformanceCounter(), SDL_GetTicks());

    int key_is_down = 0;
    int key_time_stamp = 0;

    int resolution_location = glGetUniformLocation(program, UNIFORM_RESOLUTION);
    int mouse_location      = glGetUniformLocation(program, UNIFORM_MOUSE);
    int time_location       = glGetUniformLocation(program, UNIFORM_TIME);
    int random_location     = glGetUniformLocation(program, UNIFORM_RANDOM);
    int button_location     = glGetUniformLocation(program, UNIFORM_BUTTON);

    // Set the initial value of the resolution uniform to the current width and height.
    glUniform2f(resolution_location, width, height);

    // Begin the frame loop.
    while (1) {
        // Handle any queued events.
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            } else if (event.type == SDL_MOUSEMOTION) {
                // Update the mouse uniform when the mouse has moved.
                glUniform2f(mouse_location, event.motion.x, height - event.motion.y);
            } else if (event.type == SDL_KEYDOWN) {
                if (!event.key.repeat) {
                    key_is_down = 1;
                    key_time_stamp = event.key.timestamp;
                }
            } else if (event.type == SDL_KEYUP) {
                key_is_down = 0;
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    // Update the resolution uniform when the window is resized.
                    width  = event.window.data1 * scale;
                    height = event.window.data2 * scale;
                    glUniform2f(resolution_location, width, height);
                    // Update the view port with the new resolution.
                    glViewport(0, 0, width, height);
                }
            }
        }

        // Generate a new pseudo-random number for the random uniform.
        glUniform1f(random_location, random_float());

        // Update the time uniform.
        glUniform1f(time_location, SDL_GetTicks() / 1000.0f);

        // Update the button uniform.
        if (key_is_down) {
            float time = SDL_GetTicks() - key_time_stamp;
            time /= 1000.0f;
            glUniform1f(button_location, time > 1.0f ? 1.0f : time);
        } else {
            glUniform1f(button_location, 0.0f);
        }

        // Clear the screen.
        glClear(GL_COLOR_BUFFER_BIT);

        // Render the screen-covering triangles.
        glBindVertexArray(vertex_array_object[0]);
        glDrawArrays(GL_TRIANGLES, 0, vertex_count);

        // Display the results.
        SDL_GL_SwapWindow(window);
    }
}
