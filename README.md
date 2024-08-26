# Material

I experimented with descriptors and descriptor sets to pass multiple textures to a shader. To facilitate this, I created a simple `Material` class that manages the `VkDescriptorPool` and writes descriptor sets according to the material properties it holds.

The material consists of the following maps:
- Albedo Map
- Normal Map
- ARM Map (which combines Ambient Occlusion, Roughness, and Metalness)

These maps are processed by a PBR shader, which applies different values to pixels, creating the illusion of real-world materials.

https://github.com/user-attachments/assets/308c9df7-fd33-4698-add8-17e5767e1f99

