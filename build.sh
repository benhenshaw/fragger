clang fragger.c -o fragger -framework SDL2 -Wall

if [[ $? -eq 0 ]]; then
    ./fragger creation.glsl
fi
