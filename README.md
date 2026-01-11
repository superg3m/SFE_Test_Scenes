# On going development


[![Demo Video](https://www.youtube.com/watch?v=-MbyTzapVAc/0.jpg)](https://www.youtube.com/watch?v=-MbyTzapVAc)



# Building and Run (Builds for windows, macos. Trivial to make it build for linux)
- **Bootstrap:** `./c_build/bootstrap.ps1`
- **c_build.ps1 (Release):** `./c_build.ps1 -BuildType release -Clean -Build -Run`
- **c_build.ps1 (Debug):** `./c_build.ps1 -BuildType debug -Clean -Build -Run` or `./c_build.ps1 -Debugger`

# For me:
- [x] TerrainScene
- [x] ParticleScenes (parallelism is broken for c++, but works for winapi)
- [x] TextScene (No Kerning)
- [ ] WaterScene
- [ ] FogScene
- [ ] PBRScene
- [ ] InstancedAsteroidScene
- [ ] GUIScene
- [ ] LightingScene
- [ ] TransparencyScene
- [ ] FallingSandScene: https://www.youtube.com/watch?v=HrrJxkRlRfk

```
https://wikis.khronos.org/opengl/Buffer_Object_Streaming
https://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
// make quad billboards (https://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards)
// Write a macros for speeding up the math library for examples
#if defined(PLATFORM_WINDOWS)
    #define MATH_SQRT() WINDOWS_INSTRINSIC_SQRT()
#else
    #define MATH_SQRT() sqrtf()
#endif
```