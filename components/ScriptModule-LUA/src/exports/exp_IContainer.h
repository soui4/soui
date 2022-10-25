BOOL ExpLua_IContainer(lua_State *L)
{
	try{
		lua_tinker::class_add<ISwndContainer>(L,"ISwndContainer");
		lua_tinker::class_def<ISwndContainer>(L,"GetHostHwnd",&ISwndContainer::GetHostHwnd);
		lua_tinker::class_def<ISwndContainer>(L,"IsTranslucent",&ISwndContainer::IsTranslucent);
		lua_tinker::class_def<ISwndContainer>(L,"IsSendWheel2Hover",&ISwndContainer::IsSendWheel2Hover);
		lua_tinker::class_def<ISwndContainer>(L,"GetScale",&ISwndContainer::GetScale);
		lua_tinker::class_def<ISwndContainer>(L,"GetContainerRect",&ISwndContainer::GetContainerRect);
		lua_tinker::class_def<ISwndContainer>(L,"FrameToHost",&ISwndContainer::FrameToHost);
		lua_tinker::class_def<ISwndContainer>(L,"EnableIME",&ISwndContainer::EnableIME);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}