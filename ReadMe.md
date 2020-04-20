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
  * Ray debuger - prits the full path of a single ray
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

## Reflection 
When the ray hits a reflective surface, it can bounce.

`Mirror room`
* Render time: 19 seconds
* Bounce limit: 5

![Image](https://github.com/MaksymPylypenko/Ray-Tracing/blob/master/rendered/%5B640x640%5D%20a2%2019%20sec.png)

`Mirror objects`
* Render time: 34 seconds
* Bounce limit: 5

![Image](https://github.com/MaksymPylypenko/Ray-Tracing/blob/master/rendered/%5B640x640%5D%20e%2034%20sec.png)


## Refraction
When the ray hits a refractive surface like water or diamond, it refracts. 

`Image flip`

* Antialiasing: SSAA x4
* Render time: 64 sec
* Bounce limit: 5

![Image](https://github.com/MaksymPylypenko/Ray-Tracing/blob/master/rendered/%5B640x640%5D%20b%20(x4)%2064%20sec.png)

There is also a special case where the ray would reflect instead of refracting. This is called a total internal reflection. This is what happens inside a pyramid

`Total internal reflection`
* Render time: 40 sec
* Bounce limit: 5

![Image](https://github.com/MaksymPylypenko/Ray-Tracing/blob/master/rendered/%5B640x640%5D%20g%2040%20sec.png)

## Acceleration data structure
Bounding Volume Hierarchy (BVH) for meshes significantly imrpoves the rendering time of very complex objects.

`Utah teapot`
* Triangles count: 6320
* Render time: 317 seconds = 5.3 mins
* Render time (without BVH): approximately 317 * 10 / 60  = ~53 mins

![Image](https://github.com/MaksymPylypenko/Ray-Tracing/blob/master/rendered/%5B640x640%5D%20teapot%20317%20sec.png)

## Textures

Procedural textures that generate a checkerboard pattern on the fly (e.g after a hit). 

`Checkers`
* Render time: 19 seconds
* There is aliasing on the infinite planes. An effective solution might be to add a large bounding box for the whole scene.

![Image](https://github.com/MaksymPylypenko/Ray-Tracing/blob/master/rendered/%5B640x640%5D%20c_checkers%2019%20sec.png)

There is also an ability to add custom uniform texutres (note, that objects do not carry u,v coordinates).
I used [EasyBMP](http://easybmp.sourceforge.net/)

`Custom texture`
* Render time: 19 seconds

![Image](https://github.com/MaksymPylypenko/Ray-Tracing/blob/master/rendered/%5B640x640%5D%20c_textures%2019%20sec.png)

## Constructive solid geometry (CSG)
Objects can be subtracted from each other wich may result in Subtraction / Intersection.

`Blending mode`
* render time: 15 sec 

![Image](https://github.com/MaksymPylypenko/Ray-Tracing/blob/master/rendered/%5B640x640%5D%20blending%2015%20sec.png)


## Area lighting
This type of light can create more realistic shadows, however there is a cost for that.. (need to send multiple samples to find an average). I also a simple heuristic to stop sending shadow rays after a long sequence of misses. 

* render time: 133 sec 
* max number of samples: 20
* longest sequence: 5 

![Image](https://github.com/MaksymPylypenko/Ray-Tracing/blob/master/rendered/%5B640x640%5D%20area_light%20133%20sec.png)

## Anti-Aliasing
Image quality ca also be improves using Supersampling Anti-Aliasing (SSAA) x4. This is effectively rendering the scene at higher resolution and then compressing it into a desirable resolution.

![Image](https://github.com/MaksymPylypenko/Ray-Tracing/blob/master/rendered/antialiasing%20example.png)

## Ray Debug
If you click a pixel on the scene you will receive a detailed information about the rays path. This was especially helpful for writing reflections.

```
CASTING A RAY

Triangle HIT from [Outside] @ RayLen = 2.172172,
Refracting AIR --> MATERIAL

Triangle HIT from [Inside] @ RayLen = 0.129709,
Total internal reflection ...

Triangle HIT from [Inside] @ RayLen = 0.214013,
Refracting MATERIAL --> AIR

Plane HIT from [Outside] @ RayLen = 4.186393,
```

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
 
