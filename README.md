# On going development

// [x] TerrainScnee
// WaterScene
// FogScene
// PBRScene
// ParticleScenes
// InstancedAsteroidScene
// GUIScene
// LightingScene
// TransparencyScene

// I want to make a bunch of reusable shaders
// like a simple diffuse texture shader

// make quad billboards
// Write a macros for speeding up the math library for examples
#if defined(PLATFORM_WINDOWS)
    #define MATH_SQRT() WINDOWS_INSTRINSIC_SQRT()
#else
    #define MATH_SQRT() sqrtf()
#endif