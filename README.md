# Triangle

After approximately 1000 lines of code we have set up Vulkan application to be able to create the 
one simple triangle.

Lot of lines is due to the design philosophy of Vulkan which is mainly around low driver overhead. This means that 
everything from the window and surface onto which present the images to the graphics pipeline had to be set up manually.

Triangle is rendered without having explicit `VertexArray` and triangle vertices are hard coded and accessed in shader using `gl_VertexIndex`.

Fragment shader is interpolating between colours stored in each vertex, hence the rainbow-ish look.

Additionally background is animated by smoothly caning colour during the Recording of the command buffer using `VkClear` structure and `glm::sin`
method.

The application tries to find dedicated GPU and execute program there. If system does not have one the application will instead use integrated one. By default validation layers are enabled. They can be turned off by passing `false` to the `constexpr bool enableValidationLayers` validation inside `HelloTriangle.hpp` file.

![Triangle](/VkNotes/images/VulkanTriangle.png)