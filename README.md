# Fast nearby search for boid simulation

This is a simple boid simulation with a fast nearby search algorithm. It uses the Boost Rtree to store the boids and find the nearby boids. The Rtree is a spatial index that uses a bounding box hierarchy to store the boids. The Rtree is updated every frame to reflect the current position of the boids.

![boids](screenshot.png)

## Compute Shaders

If you are on WSL you may have unsupported graphic drivers :

```
sudo add-apt-repository ppa:oibaf/graphics-drivers
sudo apt-get update
sudo apt-get dist-upgrade

sudo apt-get install libvulkan1 mesa-vulkan-drivers vulkan-utils
```

```
WARNING: dzn is not a conformant Vulkan implementation, testing use only.
WARNING: Some incorrect rendering might occur because the selected Vulkan device (Microsoft Direct3D12 (NVIDIA GeForce RTX 4070)) doesn't support base Zink requirements: feats.features.logicOp have_EXT_custom_border_color have_EXT_line_rasterization
glx: failed to create drisw screen
failed to load driver: zink
display: :0  screen: 0
direct rendering: Yes
Extended renderer info (GLX_MESA_query_renderer):
    Vendor: Microsoft Corporation (0xffffffff)
    Device: D3D12 (NVIDIA GeForce RTX 4070) (0xffffffff)
    Version: -1.1.8
    Accelerated: yes
    Video memory: 77496MB
    Unified memory: no
    Preferred profile: core (0x1)
    Max core profile version: 4.6
    Max compat profile version: 4.6
    Max GLES1 profile version: 1.1
    Max GLES[23] profile version: 3.1
OpenGL vendor string: Microsoft Corporation
OpenGL renderer string: D3D12 (NVIDIA GeForce RTX 4070)
```