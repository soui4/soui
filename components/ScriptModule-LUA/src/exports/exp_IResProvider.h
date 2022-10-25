#include <interface/SResProvider-i.h>

BOOL ExpLua_IResProvider(lua_State *L)
{
	try{
		lua_tinker::class_add<IResProvider>(L,"IResProvider");
		lua_tinker::class_inh<IResProvider,IObjRef>(L);
		lua_tinker::class_def<IResProvider>(L,"Init",&IResProvider::Init);
		lua_tinker::class_def<IResProvider>(L,"HasResource",&IResProvider::HasResource);
		lua_tinker::class_def<IResProvider>(L,"LoadIcon",&IResProvider::LoadIcon);
		lua_tinker::class_def<IResProvider>(L,"LoadBitmap",&IResProvider::LoadBitmap);
		lua_tinker::class_def<IResProvider>(L,"LoadCursor",&IResProvider::LoadCursor);
		lua_tinker::class_def<IResProvider>(L,"LoadImage",&IResProvider::LoadImage);
		lua_tinker::class_def<IResProvider>(L,"LoadImgX",&IResProvider::LoadImgX);
		lua_tinker::class_def<IResProvider>(L,"GetRawBufferSize",&IResProvider::GetRawBufferSize);
		lua_tinker::class_def<IResProvider>(L,"GetRawBuffer",&IResProvider::GetRawBuffer);
		lua_tinker::class_def<IResProvider>(L,"EnumResource",&IResProvider::EnumResource);


		return TRUE;
	}catch(...){
		return FALSE;
	}
}