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
#define CONFIG_NUM_ENTITIES 1

//-----------------------------------------------------------------------------
struct Data;

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API Data* initialize();

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API void dispose(Data* data);

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API void update(Data* data, float _dt);

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API void render(Data* data, void* _renderer);
