# RayTracer (RT)

![a 3d object made of polygons, in a box of mirrors](https://i.ibb.co/55kxqwq/ezgif-com-optimize.gif "a 3d object made of polygons, in a box of mirrors")

A raytracing program built from scratch in C language, using MinilibX minimal graphics library, with pseudo-xml files as input, a dynamic render realtime viewer, and the ability to save bmp at your resolution of choice, and the ability to render on multiple devices for faster distributed rendering.

> This project's main goal is learning raytracing from scratch, making a simple pseudo-xml parser expandable by design, and distributing computational tasks using C sockets to implement cluster rendering.

![a scene displaying the use of bump mapping to alter refraction](https://i.ibb.co/sjtxBYV/test2.png "a scene displaying the use of bump mapping to alter refraction")

## Features
- Simple shapes : sphere, cone, cylinder, ...
- Complex shapes : hyperboloid, torus, hollow cube, ...
- Shading : diffuse, specular, transparency, reflection, refraction
- Mappings : diffuse, specular, normal, transparency, reflection.
- Manipulations : plane cut, object limiting, texture cutting.
- Anti aliasing : 1x 2x 4x
- Smooth shadowing
- Parallel light
- 3d red-blue stereo rendering
- Mandelbulb fractal

## Installation
###### OSX & Linux:
`make -C ./libs/MinilibX ; make`
## Usage Example
Open the scene specified in the gui view

`./rt scene_file.xml`

Render the scene to the bmp file

`./rt --no_window save_file.bmp scene_file.xml`
## The scene file
##### Example of a scene `demo.xml`

```xml
<scene ambiant="0.5" AA="4" resolution="720" light_samples="20"></scene>
<camera position="(10, 20, 80)" lookat="(0, 20, 0)" fov="40"></camera>
<light center="(0, 200, 20)"  radius="3" intensity="0.5" color="#FFFFFF"></light>
<light center="(0, 100, 100)"  radius="1" intensity="0.5" color="#FFFFFF"></light>
<cone length="30" center="(0, 10, 0)" color="#FF00FF" axis="(0, 1, 0)" radius="6"></cone>
<plane center="(0, -1, -10)" length="(60, 60)" U="(0, 1, 0)" V="(1, 0, 0)"  color="#D3D3D3"></plane>
 <plane center="(0, -1, 0)" length="(60, 60)"  U="(0, 0, 1)" V="(1, 0, 0)"  color="#D3D3D3"></plane>
<sphere center="(5, 10, 50)" color="#FF0000" radius="3"></sphere>
<ellipsoid center="(30, 3, -10)" axis="(10,5, 5)" translation="(3, 0, 0)" color="#000000" radius="4"></ellipsoid>
```
I invite you to discover more about the available tags and properties from the source code and the example scenes in `rt-scenes`
##### Result
![Rendering result of the previous example](https://i.ibb.co/vkDzwSF/demo.png "Rendering result of the example above")
## The GUI mode
The GUI mode gives you the freedom of moving the camera, enabling and disabling some effects and saving the result to a bmp.

![the GUI](https://i.ibb.co/7SbPBLL/Screen-Shot-2020-05-17-at-1-11-51-AM.png "the GUI")
## Some Renderings
![light going thru a cut object](https://i.ibb.co/GJwD6G3/158967922388267.png "light going thru a cut object")
![light going thru cut object, but with soft shadows on](https://i.ibb.co/hHWwqKY/diapositive.png "light going thru cut object, but with soft shadows on")
![inside a transparent refractive sphere](https://i.ibb.co/QMSt2Qr/1580384043170700.png "inside a transparent refractive sphere")
![partially transparent sphere with earth normal mapping](https://i.ibb.co/y4t46qq/1580384119194446.png "partially transparent sphere with earth normal mapping")
![Simple scene with reflection](https://i.ibb.co/1rgdcb0/1583013314639682.png "Simple scene with reflection")
![scene with procedurally generator textures](https://i.ibb.co/CbWGkH4/1589678433499213.png "scene with procedurally generator textures")
## Authors
[MbarkErras](https://github.com/MbarkErras "github.com/MbarkErras")

[KernelOverseer](https://github.com/KernelOverseer "github.com/KernelOverseer")

[abenaiss](https://github.com/abenaiss "github.com/abenaiss")

[abdzr](https://github.com/abdzr "https://github.com/abdzr")
