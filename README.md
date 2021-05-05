<div align="center">
    <img src="https://github.com/Romop5/holoinjector/raw/master/media/logo.png" alt="Logo with anaglyph motive" height="100px"/>
<br>
<h1>HoloInjector - See things in real 3D </h1> 
</div>
<hr>

## About
*This software was created as a part of Master's thesis at [FIT VUT](https://www.fit.vut.cz/.en).*

A prototype of generic single-view 3D to multiview convertor. In principle, the conventor intercepts OpenGL API calls and attempts
to replicate dispatched geometry with specific transformation into multiple views.

## When would I use this tool
Consider you have just bought a display from [Looking Glass Factory](https://lookingglassfactory.com/), but you your favorite application is not supporting it. 
In theory, you could install this conversion layer and run the app as-it-is on the display. In practise, this tool
supports a subset of OpenGL-based 3D applications and some of them may require manual tweekings.

Currently, this tool is directly outputing Looking Glass's native format. 

See [holoinjector-tests](https://github.com/Romop5/holoinjector-tests/) for examples of converted OpenGL applications to the quilt representation.

## Features
- intercepting draw calls & dispatching duplicated draw calls 
- automated detection of MVP or VP matrix uniforms in pipeline
- almost 100% support of fixed-pipeline apps

## Limitations

- Linux & XServer & OpenGL only

## Documentation
- Link to master's thesis: SOON
- Link to paper (Excel@FIT VUT): http://excel.fit.vutbr.cz/submissions/2021/018/18.pdf
- Additionally, you can use Doxygen to generate an HTML representation of docs.

# Bulding & Installation

A reasonable up-to-day version of CMake is required for building.

Use *cloneDeps.sh* script to get 3rd party dependencies \& create a CMake configuration
ready-to-compile.

**NOTE**: Usage of the script above is highly recommended as some of dependencies must be configured
& extended with CMake command, and this is done in the script.

Upon successful run of the script, build is generated in *build* directory.

## Dependencies
- [subhook](https://github.com/Romop5/subhook) - for run-time code patching
- [simplecpp](https://github.com/Romop5/simplecpp)  - for preprocessing GLSL shaders
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) - for configuration
- [imgui](https://github.com/ocornut/imgui) - for debugging / in-app menu

## Future plans

In case you are interested in further development of this tool, consider writing me directly in
person.

## Author
- Roman [Romop5](https://github.com/Romop5) Dobias (xdobia11(at)stud.fit.vutbr.cz)
## Supervised by
- Ing. Tomas [dormon](https://github.com/dormon) Milet (imilet(at)fit.vutbr.cz)

