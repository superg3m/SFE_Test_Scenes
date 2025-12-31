# On going development

// [x] TerrainScnee
// WaterScene
// FogScene
// PBRScene
// [x] ParticleScenes
// InstancedAsteroidScene
// GUIScene
// LightingScene
// TransparencyScene
// FallingSandScene: https://www.youtube.com/watch?v=HrrJxkRlRfk

// I want to make a bunch of reusable shaders
// like a simple diffuse texture shader

https://wikis.khronos.org/opengl/Buffer_Object_Streaming
https://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
// make quad billboards (https://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards)
// Write a macros for speeding up the math library for examples
#if defined(PLATFORM_WINDOWS)
    #define MATH_SQRT() WINDOWS_INSTRINSIC_SQRT()
#else
    #define MATH_SQRT() sqrtf()
#endif