# PicoRender

*This code is Obsolete*. It was written by me around 2014, when I just started doing graphics.

Very simple raytracer for baking ambient occlusion and [bent normal](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.230.6374&rep=rep1&type=pdf). No dependencies.



Now, it does not have much of a value, but I did minor fixes/cleanup out of curiosity to dig some of my old code.

The main purpose was to produce ambient occlusion maps and [bent normal](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.230.6374&rep=rep1&type=pdf) maps.



|   |           Camera space   |           Texture space  |
:---|:------------------------:|:-------------------------:
Albido | ![](example_images/example_orthogonal_projection_albido.png)  |  ![](example_images/example_texture_space_albido.png)
Ambient Occlusion / Lightmap| ![](example_images/example_orthogonal_projection_gi.png)  |  ![](example_images/example_texture_space_gi.png)
Normals | ![](example_images/example_orthogonal_projection_normal.png)  |  ![](example_images/example_texture_space_normal.png)
Bent Normals (AO in alpha)| ![](example_images/example_orthogonal_projection_gi_normal.png)  |  ![](example_images/example_texture_space_gi_normal.png)
Composition | ![](example_images/example_orthogonal_projection_composed.png)  |  ![](example_images/example_texture_space_composed.png)



|   |           Camera space   |           Texture space  |
:---|:------------------------:|:-------------------------:
Total Ambient Occlusion / Lightmap| ![](example_images/example_orthogonal_projection_gi.png)  |  ![](example_images/example_texture_space_gi.png)
Bounce 1 | ![](example_images/example_orthogonal_projection_gi_by_bounce1.png)  |  ![](example_images/example_texture_space_gi_by_bounce1.png)
Bounce 2 | ![](example_images/example_orthogonal_projection_gi_by_bounce2.png)  |  ![](example_images/example_texture_space_gi_by_bounce2.png)
Bounce 3 | ![](example_images/example_orthogonal_projection_gi_by_bounce3.png)  |  ![](example_images/example_texture_space_gi_by_bounce3.png)
Bounce 4 | ![](example_images/example_orthogonal_projection_gi_by_bounce4.png)  |  ![](example_images/example_texture_space_gi_by_bounce4.png)
