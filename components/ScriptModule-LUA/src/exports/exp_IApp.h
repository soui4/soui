#include <SApp.h>


BOOL ExpLua_IAppication(lua_State *L)
{
	try{
		lua_tinker::class_add<IApplication>(L,"IApplication");
		lua_tinker::class_inh<IApplication,IObjRef>(L);
		lua_tinker::class_def<IApplication>(L,"GetModule",&IApplication::GetModule);

		lua_tinker::class_def<IApplication>(L,"LoadSystemNamedResource",&IApplication::LoadSystemNamedResource);
		lua_tinker::class_def<IApplication>(L,"GetTranslator",&IApplication::GetTranslator);
		lua_tinker::class_def<IApplication>(L,"SetTranslator",&IApplication::SetTranslator);
		lua_tinker::class_def<IApplication>(L,"GetToolTipFactory",&IApplication::GetToolTipFactory);
		lua_tinker::class_def<IApplication>(L,"SetToolTipFactory",&IApplication::SetToolTipFactory);
		lua_tinker::class_def<IApplication>(L,"SetMsgLoopFactory",&IApplication::SetMsgLoopFactory);
		lua_tinker::class_def<IApplication>(L,"GetMsgLoopFactory",&IApplication::GetMsgLoopFactory);
		lua_tinker::class_def<IApplication>(L,"SetLogManager",&IApplication::SetLogManager);
		lua_tinker::class_def<IApplication>(L,"GetLogManager",&IApplication::GetLogManager);
		lua_tinker::class_def<IApplication>(L,"SetAttrStorageFactory",&IApplication::SetAttrStorageFactory);
		lua_tinker::class_def<IApplication>(L,"GetAttrStorageFactory",&IApplication::GetAttrStorageFactory);
		lua_tinker::class_def<IApplication>(L,"Run",&IApplication::Run);
		lua_tinker::class_def<IApplication>(L,"Quit",&IApplication::Quit);

		lua_tinker::class_def<IApplication>(L,"GetMainWnd",&IApplication::GetMainWnd);
		lua_tinker::class_def<IApplication>(L,"AddMsgLoop",&IApplication::AddMsgLoop);
		lua_tinker::class_def<IApplication>(L,"RemoveMsgLoop",&IApplication::RemoveMsgLoop);
		lua_tinker::class_def<IApplication>(L,"GetMsgLoop",&IApplication::GetMsgLoop);
		lua_tinker::class_def<IApplication>(L,"GetResProviderMgr",&IApplication::GetResProviderMgr);
        lua_tinker::class_def<IApplication>(L,"SetRealWndHandler",&IApplication::SetRealWndHandler);
		lua_tinker::class_def<IApplication>(L,"GetRenderFactory",&IApplication::GetRenderFactory);
		lua_tinker::class_def<IApplication>(L,"SetScriptFactory",&IApplication::SetScriptFactory);
		lua_tinker::class_def<IApplication>(L,"InitXmlNamedID",&IApplication::InitXmlNamedID);
		lua_tinker::class_def<IApplication>(L,"LoadXmlDocment",&IApplication::LoadXmlDocment);
		lua_tinker::class_def<IApplication>(L,"LoadAnimation",&IApplication::LoadAnimation);
		lua_tinker::class_def<IApplication>(L,"LoadValueAnimator",&IApplication::LoadValueAnimator);
		lua_tinker::class_def<IApplication>(L,"EnableNotifyCenter",&IApplication::EnableNotifyCenter);

		lua_tinker::class_def<IApplication>(L,"GetInnerSingleton",&IApplication::GetInnerSingleton);
		lua_tinker::class_def<IApplication>(L,"CreateObject",&IApplication::CreateObject);
		lua_tinker::class_def<IApplication>(L,"SetCreateObjectCallback",&IApplication::SetCreateObjectCallback);


		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}