
#ifndef OZZ_C_WRAPPER
#define OZZ_C_WRAPPER

//-----------------------------------------------------------------------------
#if defined(__cplusplus)
#	define OZZ_ANIMATION_C_API extern "C"
#else
#	define OZZ_ANIMATION_C_API
#endif // defined(__cplusplus)

//-----------------------------------------------------------------------------
#define CONFIG_MAX_SKELETONS 2
#define CONFIG_MAX_ANIMATIONS 2
#define CONFIG_MAX_MESHS 2
#define CONFIG_MAX_TEXTURES 2
#define CONFIG_MAX_ENTITIES 2

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API struct EntityConfig
{
  unsigned int skeletonId;
  unsigned int animationId;
  unsigned int meshId;
  unsigned int textureId;
  float* transform;
  float timeOffset;
};

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API struct Config
{
  char* skeletonPaths[CONFIG_MAX_SKELETONS];   // optionnal \0 to mark the last skeleton path of the list
  char* animationPaths[CONFIG_MAX_ANIMATIONS]; // optionnal \0 to mark the last animation path of the list
  char* meshsPaths[CONFIG_MAX_MESHS];          // optionnal \0 to mark the last mesh path of the list
  char* texturesPaths[CONFIG_MAX_TEXTURES];    // optionnal \0 to mark the last texture path of the list
  EntityConfig* entities;
  unsigned int entitiesCount;
};

//-----------------------------------------------------------------------------
static float defaultTransformIdentity[16] = {1.0f, 0.0f, 0.0f, 0.0f,
                                             0.0f, 1.0f, 0.0f, 0.0f,
                                             0.0f, 0.0f, 1.0f, 0.0f,
                                             0.0f, 0.0f, 0.0f, 1.0f};
static EntityConfig defaultEntitiesConfig[] = { {0, 0, 0, 0, defaultTransformIdentity, 0.0f}, {1, 1, 1, 1, defaultTransformIdentity, 7.0f} };
static Config defaultConfiguration = {
                                       {"media/alain_skeleton.ozz", "media/ExportPersoRue01_skeleton.ozz"},
                                       {"media/alain_atlas.ozz", "media/ExportPersoRue01_animation.ozz"},
                                       {"media/arnaud_mesh.ozz", "media/ExportPersoRue01_mesh.ozz"},
                                       {"media/UVW_man00.jpg", "media/UVW_man01_b.jpg"},
                                       defaultEntitiesConfig,
                                       sizeof(defaultEntitiesConfig) / sizeof(EntityConfig)
                                     };

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API struct Data;

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API Data* initialize(Config* config);

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API void dispose(Data* data);

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API void update(Data* data, float _dt);

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API void render(Data* data, float* viewProjMatrix);

#endif // OZZ_C_WRAPPER
