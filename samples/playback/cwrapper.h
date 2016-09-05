//-----------------------------------------------------------------------------
#if defined(__cplusplus)
#	define OZZ_ANIMATION_C_API extern "C"
#else
#	define OZZ_ANIMATION_C_API
#endif // defined(__cplusplus)

//-----------------------------------------------------------------------------
struct Data;

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API Data* initialize();

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API void dispose(Data* data);

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API bool update(Data* data, float _dt);

//-----------------------------------------------------------------------------
OZZ_ANIMATION_C_API bool render(Data* data, void* _renderer);
