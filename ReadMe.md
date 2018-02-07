# Fragger
Runs a fragment shader across the whole screen.

```
fragger [-r] [file.glsl]
```

| Argument | Description |
| ---      | ---         |
| **\***   | The first argument without a '-' will be treated as the shader file to read. |
| **-d**   | Print debuf info. |
| **-r**   | Retina display mode (2x scale). |

Uses [SDL2](https://libsdl.org) and [glad](http://glad.dav1d.de/).

An [example shader](https://github.com/benhenshaw/fragger/blob/master/creation.glsl) is provided.