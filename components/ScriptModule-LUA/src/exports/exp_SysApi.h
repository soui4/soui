#include <windows.h>
#include <SPlaySound.h>

// PlaySound(path[, nostop]):播放 wav 音效(异步播放)。
// path:wav 文件绝对路径,仅支持文件播放(SND_FILENAME)。
// nostop:可选,缺省 true —— 带 SND_NOSTOP,已有音效在播就不打断它。wasm 宿主按
//   Win32 语义实现该标志(有一路在响就丢弃新的这条);Windows 的 AudioPlayer 自己
//   混音、标志无效。需要"炮声压过背景音效"的游戏(如坦克大战)传 false,让多条叠加。
// 返回:true=已提交播放(异步,不代表声音已播完);false=失败或被 NOSTOP 丢弃。
static int Lua_PlaySound(lua_State *L)
{
	const char *path = lua_tostring(L, 1);
	if (path == NULL || path[0] == '\0')
		return 0;
	int bNoStop = (lua_gettop(L) >= 2) ? lua_toboolean(L, 2) : 1;
	SStringT str = S_CA2T(path,CP_UTF8);
	DWORD dwFlags = SND_FILENAME | SND_ASYNC | (bNoStop ? SND_NOSTOP : 0);
	BOOL bRet = SPlaySound(str, NULL, dwFlags);
	lua_pushboolean(L, bRet ? 1 : 0);
	return 1;
}

// GetAppDir():取程序资源目录(demo 的 getSourceDir:macOS=.app 包根目录,
// 其余平台=exe 所在目录下的 demo_res)。lua 侧用它拼接 snd 等外部文件
// 资源的绝对路径。
// 返回:目录字符串(不带结尾分隔符,拼接时自行补 '/')。
static int Lua_GetAppDir(lua_State *L)
{
	SStringA dir = S_CT2A(SApplication::getSingleton().GetAppDir(),CP_UTF8);
	lua_pushstring(L, dir.c_str());
	return 1;
}

BOOL ExpLua_SysApi(lua_State *L)
{
	try{

		lua_tinker::def(L,"PostQuitMessage",PostQuitMessage);
		lua_tinker::def(L,"GetActiveWindow",GetActiveWindow);
		lua_tinker::def(L,"PlaySound",Lua_PlaySound);
		lua_tinker::def(L,"GetAppDir",Lua_GetAppDir);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}

}
