#include <interface/SResProviderMgr-i.h>

BOOL ExpLua_IResProviderMgr(lua_State *L)
{
	try{
		lua_tinker::class_add<IResProviderMgr>(L,"IResProviderMgr");
		lua_tinker::class_def<IResProviderMgr>(L,"AddResProvider",&IResProviderMgr::AddResProvider);
		lua_tinker::class_def<IResProviderMgr>(L,"RemoveResProvider",&IResProviderMgr::RemoveResProvider);
		lua_tinker::class_def<IResProviderMgr>(L,"RemoveAll",&IResProviderMgr::RemoveAll);
		lua_tinker::class_def<IResProviderMgr>(L,"GetHeadResProvider",&IResProviderMgr::GetHeadResProvider);
		lua_tinker::class_def<IResProviderMgr>(L,"GetTailResProvider",&IResProviderMgr::GetTailResProvider);
		lua_tinker::class_def<IResProviderMgr>(L,"SetFilePrefix",&IResProviderMgr::SetFilePrefix);


		lua_tinker::class_def<IResProviderMgr>(L,"HasResource",&IResProviderMgr::HasResource);
		lua_tinker::class_def<IResProviderMgr>(L,"LoadIcon",&IResProviderMgr::LoadIcon);
		lua_tinker::class_def<IResProviderMgr>(L,"LoadBitmap",&IResProviderMgr::LoadBitmap);
		lua_tinker::class_def<IResProviderMgr>(L,"LoadCursor",&IResProviderMgr::LoadCursor);
		lua_tinker::class_def<IResProviderMgr>(L,"LoadImage",&IResProviderMgr::LoadImage);
		lua_tinker::class_def<IResProviderMgr>(L,"LoadImgX",&IResProviderMgr::LoadImgX);
		lua_tinker::class_def<IResProviderMgr>(L,"GetRawBufferSize",&IResProviderMgr::GetRawBufferSize);
		lua_tinker::class_def<IResProviderMgr>(L,"GetRawBuffer",&IResProviderMgr::GetRawBuffer);


		return TRUE;
	}catch(...){
		return FALSE;
	}
}