# On going development

<video src="./Assets/Videos/demo.mp4" width="640" height="480" controls></video>

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