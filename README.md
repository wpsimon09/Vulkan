# Vulkan

Simple repository where I document progress while learning the [Vulkan API](https://www.vulkan.org/).

Files and their meagning:

- `Cmaera.hpp & cpp` - class representing orbit camera that contains calculations of Projection and View metrices 
- `Material.hpp & cpp` - class representing single material formed by 3 textures, namely Albedo, Arm and Normal. It creates descriptor pools to allocate descriptors from as well as other useful abstraction
- `DebugInfoLog.hpp` - header file for more structured validation errors provided by Vulkan validation layer.
- `Structs.hpp` - definitions of structures and enums for stuff like `Vertex`, `UnifromBufferObjects` and `GeometryType`
- `Utils.hpp` - functions that are not directly related to the rendering. One can here find functions for picking right physical device, surface, presentation layer etc.
- `VertexData.hpp` - contains definitions for Vertex array and index array of various geometry packed together in `VertexData` namespace 
- `VulkanApp.hpp & .cpp` - all Vulkan related stuff. From `vkInstance` creation to Swap chain presentation. Due to the Vulkan design it contains roughly 1500 lines of code.
- `Shaders/compile.sh` - bash script that compiles every vertex and fragment shader and puts them to the `Compiled` directory created by the script. Compiled shaders are in SPIR-V format.
- `main.cpp` - app instantiation 
- `VkNotes` - directory that contains Obsidian vault with all my notes

## Branches

Different branches contain important milestones like first triangle and some experiments like PBR shading

