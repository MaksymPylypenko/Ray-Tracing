# Ray tracing 

This is a recursive ray tracer that I built as a final project for my Computer Graphics course.

<details>
 <summary>A complete list of features</summary>
<p>
    
* `Basic ray tracer`
  * Bidirectional hit tests for [Sphere, Plane, Triangle]
  * Phong illumination 
  * Light sources: [Ambient, Point, Direction, Spot, Areal]
  * Ray functions: Reflection, Refraction (or simple Transmission)
  * Recursive trace function - finds the closest hit
  * Shadow trace function - finds the first hit
* `Acceleration`
  * Axis-Aligned Bounding Box (AABB) intersection test
  * Acceleration data structure (BVH) for Meshes
* `Transformations`
  * Mesh translation
  * Mesh scaling
  * Mesh rotation (e.g. using axis & angle)
* `Quality`
  * Procedural texture mapping (e.g. checkerboard pattern).
  * Custom texture mapping from a BMP file.
* `New Geometry`
  * Union
  * Intersection
  * Subtraction
* `Techniques`
  * Antialisaing (x4)
  * Areal lighting
</details>




# How it was built? Good & Bad decisions..

@ TODO

![Image](https://github.com/MaksymPylypenko/Ray-Tracing/blob/master/rendered/a.png)

![Image](https://github.com/MaksymPylypenko/Ray-Tracing/blob/master/rendered/b.png)


## PC specs 
The program was successfully tested using:
* Processor: Intel(R) Core(TM) i7-2630QM CPU @ 2.00GHz
* RAM: 8.00GB
* OS: 64-bit Windows 10
* IDE: Visual Studio 2019 16.4
* Graphics card: AMD Radeon HD 6700M Series
  * Memory size 1024 MB
  * Memory type GDDR5
  * Total Memory Bandwidth 51.2 GBytes/s
 
