# PicoRender

Very simple raytracer for baking ambient occlusion and [bent normal](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.230.6374&rep=rep1&type=pdf). No dependencies.

This code was written by me around 2014, when I just started doing graphics.

Now, it does not have much of a value, but I did minor fixes/cleanup out of curiosity to dig some of my old code.

The main purpose was to produce ambient occlusion maps and [bent normal](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.230.6374&rep=rep1&type=pdf) maps.



|   |           Camera space   |           Texture space  |
:---|:------------------------:|:-------------------------:
Albido | ![](example_images/example_camera_diff0.png)  |  ![](example_images/example_texture_space_diff0.png)
Ambient Occlusion | ![](example_images/example_camera_gi0.png)  |  ![](example_images/example_texture_space_gi0.png)
Normals | ![](example_images/example_camera_normal.png)  |  ![](example_images/example_texture_space_normal.png)
Bent Normals (AO in alpha)| ![](example_images/example_camera_gi_normal0.png)  |  ![](example_images/example_texture_space_gi_normal0.png)
Composition | ![](example_images/example_camera_composed.png)  |  ![](example_images/example_texture_space_composed.png)
