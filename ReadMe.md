# Fragger
Runs a fragment shader across the whole screen.

```
fragger [args] [file.glsl]
```

Uses [SDL2](https://libsdl.org) and [glad](http://glad.dav1d.de/).

An [example shader](https://github.com/benhenshaw/fragger/blob/master/creation.glsl) is provided.

| Argument | Description |
| ---      | --- |
| **\***   | The first argument without a '-' will be treated as the shader file to read. If none is provided fragger will attempt to open the file 'frag.glsl' in the current directory. |
| **-d**   | Print debug info. |
| **-r**   | Retina display mode (2x scale). |

#### Uniforms

| Name           | type  | Description |
| ---            | ---   | --- |
| **resolution** | vec2  | The width and height of the window. |
| **mouse**      | vec2  | The x and y position of the mouse. |
| **time**       | float | The number of seconds since the program launched. |
