//-----------------------------------------------------------------------------
#if defined(__cplusplus)
#	define OZZ_ANIMATION_C_API extern "C"
#else
#	define OZZ_ANIMATION_C_API
#endif // defined(__cplusplus)

//-----------------------------------------------------------------------------
#define CONFIG_NUM_SKELETONS 1
#define CONFIG_NUM_ANIMATIONS 1
#define CONFIG_NUM_MESHS 1
#define CONFIG_NUM_ENTITIES 2

//-----------------------------------------------------------------------------
struct EntityConfig
{
  unsigned int skeletonId;
  unsigned int animationId;
  unsigned int meshId;
  float* transform;
};

//-----------------------------------------------------------------------------
struct Config
{
  char* skeletonPaths[CONFIG_NUM_SKELETONS];
  char* animationPaths[CONFIG_NUM_ANIMATIONS];
  char* meshsPaths[CONFIG_NUM_MESHS];
  EntityConfig entities[CONFIG_NUM_ENTITIES];
};

//-----------------------------------------------------------------------------
static Config defaultConfiguration = { {"media/alain_skeleton.ozz"},
                                       {"media/alain_atlas.ozz"},
                                       {"media/arnaud_mesh.ozz"},
                                       {{0, 0, 0, 0}, {0, 0, 0, 0}} };

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
