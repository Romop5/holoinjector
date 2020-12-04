# visionEnhancer {#mainpage}
### See things in real 3D

## About
A prototype of generic 3D-to-multiview3D convertor. In principle, the conventor intercepts OpenGL API calls and attempts
to replicate dispatched geometry with specific transformation into multiple views.

# Idea of conversion
- duplicate draw calls so that the same view frustum is rendered multiple times with slightly
shifted camera's translation
- to do so, Vertex Shader is hooked and processed to allow shifting

# Project's architecture
TODO

# Bulding && Installation

**Pro tip:** use *cloneDeps.sh* script to get 3rd party dependencies \& create a CMake configuration
ready-to-compile.

## Author
- Roman Dobias (xdobia11(at)stud.fit.vutbr.cz)
## Supervised by
- Ing. Tomas Milet (imilet(at)fit.vutbr.cz)

