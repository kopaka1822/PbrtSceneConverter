# PbrtSceneConverter
Project that converts pbrt scene files into c++ classes => can be used to write an exporter to another scene format.

The project is configured as a console application that takes an input pbrt file, the output destination and some optional parameters. 
See [Source/main.cpp](https://github.com/kopaka1822/PbrtSceneConverter/blob/master/Source/main.cpp) for the description of all optional parameters.

To convert the PBRT c++ class to you own format, replace the `// TODO convert to your own scene format here` in `main()` with your converter.

The most important PBRT c++ classes are:
- [PbrtScene](https://github.com/kopaka1822/PbrtSceneConverter/blob/master/Source/PBRT/PbrtScene.h) was used to build the Scene.
Call `getRenderOptions()` to get the scene configuation.
- RenderOptions:
```c++
struct RenderOptions
{
  Matrix cameraToWorld;
  SceneObject camera;
  Vector cameraPos;
  Vector cameraLookAt;
  SceneObject sampler;
  SceneObject film;
  SceneObject renderer;
  SceneObject surfaceIntegrator;
  SceneObject volumeIntegrator;
  SceneObject accelerator;
  SceneObject pixelFilter;
  std::vector<SceneObject> volumeRegions;
  std::vector<std::unique_ptr<Shape>> shapes;
  std::vector<Light> lights;
};
```
- SceneObject. Used to describe the Camera, Renderer etc.
```c++
	struct SceneObject
	{
		std::string type;
		ParamSet set;
	};
```
- [ParamSet](https://github.com/kopaka1822/PbrtSceneConverter/blob/master/Source/PBRT/ParamSet.h) Dictionary to look up parameters. 
Example: If the Camera SceneObject has the type "perspective", you can use `camera.set.getFloat("fov", 90.0f)` to retrieve the fov. If the parameter was not set, `90.0f` will be returned.

- [Shape](https://github.com/kopaka1822/PbrtSceneConverter/blob/master/Source/geometry/Shape.h): base class for all shapes. Implemented shapes are: 
[Triangle](https://github.com/kopaka1822/PbrtSceneConverter/blob/master/Source/geometry/TriangleMesh.h) and
[Sphere](https://github.com/kopaka1822/PbrtSceneConverter/blob/master/Source/geometry/Sphere.h)

- [Light](https://github.com/kopaka1822/PbrtSceneConverter/blob/master/Source/PBRT/Light.h)
- [Material](https://github.com/kopaka1822/PbrtSceneConverter/blob/master/Source/PBRT/Material.h)
