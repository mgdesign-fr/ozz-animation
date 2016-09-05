//-----------------------------------------------------------------------------
#if defined(__cplusplus)
#	define OZZ_ANIMATION_C_API extern "C"
#else
#	define OZZ_ANIMATION_C_API
#endif // defined(__cplusplus)

//-----------------------------------------------------------------------------
#define CONFIG_MAX_SKELETONS 1
#define CONFIG_MAX_ANIMATIONS 1
#define CONFIG_MAX_MESHS 1
#define CONFIG_MAX_ENTITIES 2

//-----------------------------------------------------------------------------
struct EntityConfig
{
  unsigned int skeletonId;
  unsigned int animationId;
  unsigned int meshId;
  float* transform;
  float timeOffset;
};

//-----------------------------------------------------------------------------
struct Config
{
  char* skeletonPaths[CONFIG_MAX_SKELETONS + 1];   // \0 to mark the last skeleton path of the list
  char* animationPaths[CONFIG_MAX_ANIMATIONS + 1]; // \0 to mark the last animation path of the list
  char* meshsPaths[CONFIG_MAX_MESHS + 1];          // \0 to mark the last mesh path of the list
  EntityConfig* entities;
  unsigned int entitiesCount;
};

//-----------------------------------------------------------------------------
static float defaultTransformIdentity[16] = {1.0f, 0.0f, 0.0f, 0.0f,
                                             0.0f, 1.0f, 0.0f, 0.0f,
                                             0.0f, 0.0f, 1.0f, 0.0f,
                                             0.0f, 0.0f, 0.0f, 1.0f};
static EntityConfig defaultEntitiesConfig[] = { {0, 0, 0, defaultTransformIdentity, 0.0f}, {0, 0, 0, defaultTransformIdentity, 7.0f} };
static Config defaultConfiguration = {
                                       {"media/alain_skeleton.ozz", 0},
                                       {"media/alain_atlas.ozz", 0},
                                       {"media/arnaud_mesh.ozz", 0},
                                       defaultEntitiesConfig,
                                       sizeof(defaultEntitiesConfig) / sizeof(EntityConfig)
                                     };

//-----------------------------------------------------------------------------
struct Data;

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API Data* initialize(Config* config);

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API void dispose(Data* data);

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API void update(Data* data, float _dt);

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API void render(Data* data, void* _renderer);
