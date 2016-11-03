
#ifndef OZZ_C_WRAPPER
#define OZZ_C_WRAPPER

//-----------------------------------------------------------------------------
#ifdef CAPI_BUILD_STATIC
# ifdef __cplusplus
#   define OZZ_ANIMATION_C_API extern "C"
# else
#   define OZZ_ANIMATION_C_API
# endif // __cplusplus
#else
# ifdef CAPI_BUILD_DLL
#	  define OZZ_ANIMATION_C_API extern "C" __declspec(dllexport)
# else
#   ifdef __cplusplus
#     define OZZ_ANIMATION_C_API extern "C" __declspec(dllimport)
#   else
#     define OZZ_ANIMATION_C_API __declspec(dllimport)
#   endif // __cplusplus
# endif // CAPI_BUILD_DLL
#endif // CAPI_BUILD_STATIC

//-----------------------------------------------------------------------------
#define CONFIG_MAX_SKELETONS 6
#define CONFIG_MAX_ANIMATIONS 6
#define CONFIG_MAX_MESHS 6
#define CONFIG_MAX_TEXTURES 18
#define CONFIG_MAX_ENTITIES 16

//-----------------------------------------------------------------------------
struct EntityConfig
{
  unsigned int skeletonId;
  unsigned int animationId;
  unsigned int meshId;
  unsigned int textureId;
  float* transform;
  float timeOffset;
};

//-----------------------------------------------------------------------------
struct Config
{
  char* skeletonPaths[CONFIG_MAX_SKELETONS];   // optionnal \0 to mark the last skeleton path of the list
  char* animationPaths[CONFIG_MAX_ANIMATIONS]; // optionnal \0 to mark the last animation path of the list
  char* meshsPaths[CONFIG_MAX_MESHS];          // optionnal \0 to mark the last mesh path of the list
  char* texturesPaths[CONFIG_MAX_TEXTURES];    // optionnal \0 to mark the last texture path of the list
  struct EntityConfig* entities;
  unsigned int entitiesCount;
};

//-----------------------------------------------------------------------------
static float defaultTransformIdentity[16] = {1.0f, 0.0f, 0.0f, 0.0f,
                                             0.0f, 1.0f, 0.0f, 0.0f,
                                             0.0f, 0.0f, 1.0f, 0.0f,
                                             0.0f, 0.0f, 0.0f, 1.0f};
static struct EntityConfig defaultEntitiesConfig[] = { {1, 1, 1, 1, defaultTransformIdentity, 0.0f}};
static struct Config defaultConfiguration = {
                                              {"media/Man01a_ProOptimized(TalkLoop_0-200)_skeleton.ozz", "media/Man02b_ProOptimized(StandLoop_0-200)_skeleton.ozz"},
                                              {"media/Man01a_ProOptimized(TalkLoop_0-200)_animation.ozz", "media/Man02b_ProOptimized(StandLoop_0-200)_animation.ozz"},
                                              {"media/Man01a_ProOptimized(TalkLoop_0-200)_mesh.ozz", "media/Man02b_ProOptimized(StandLoop_0-200)_mesh.ozz"},
                                              {"media/UVW_man00.jpg", "media/UVW_man00.jpg"},
                                              defaultEntitiesConfig,
                                              sizeof(defaultEntitiesConfig) / sizeof(struct EntityConfig)
                                            };

//-----------------------------------------------------------------------------
struct Data;

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API struct Data* initialize(struct Config* config);

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API void dispose(struct Data* data);

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API void update(struct Data* data, float _dt);

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API void setEntityTransform(struct Data* data, unsigned int entityId, float* transform);

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API void render(struct Data* data, float* viewProjMatrix, int position_attrib, int normal_attrib, int uv_attrib, int u_model_matrix, int u_view_projection_matrix, int u_texture);
// OZZ_ANIMATION_C_API void render(struct Data* data, float* viewProjMatrix); // CAPI_NO_SHADER

#endif // OZZ_C_WRAPPER
