# Pathtracer

This is my toy monte-carlo path tacer.

## Features

-   Obj loading
-   Textures
-   Environment maps
-   Dielectrics (refraction and caustics)
-   Emissive materials
-   Bounding volume hierarchy
-   Depth of field

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Results

![](https://www.jakobmaier.at/posts/fun-with-path-tracing/img/bmw_m3_e30.png)

![](./doc/results/render_d94b533_1024_3_25-11-2024_202142.png)

![](./doc/results/render_5000s_3b_1731181914.png)

![](./doc/results/render_5000s_3b_1731060148.png)

![](./doc/results/cornell_box_10000s_3b_1730735703.png)

![](./doc/results/bunny_10000s_3b.png)
