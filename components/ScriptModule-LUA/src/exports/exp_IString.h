#include <interface/sstring-i.h>

BOOL ExpLua_IStringA(lua_State *L)
{
	try{

		lua_tinker::class_add<IStringA>(L,"IStringA");
		lua_tinker::class_inh<IStringA,IObjRef>(L);

		lua_tinker::class_def<IStringA>(L,"GetLength",&IStringA::GetLength);
		lua_tinker::class_def<IStringA>(L,"IsEmpty",&IStringA::IsEmpty);
		lua_tinker::class_def<IStringA>(L,"Empty",&IStringA::Empty);
		lua_tinker::class_def<IStringA>(L,"GetAt",&IStringA::GetAt);
		lua_tinker::class_def<IStringA>(L,"SetAt",&IStringA::SetAt);

		lua_tinker::class_def<IStringA>(L,"c_str",&IStringA::c_str);
		lua_tinker::class_def<IStringA>(L,"Compare",&IStringA::Compare);
		lua_tinker::class_def<IStringA>(L,"CompareNoCase",&IStringA::CompareNoCase);

		lua_tinker::class_def<IStringA>(L,"TrimBlank",&IStringA::TrimBlank);
		lua_tinker::class_def<IStringA>(L,"InsertChar",&IStringA::InsertChar);
		lua_tinker::class_def<IStringA>(L,"Insert",&IStringA::Insert);
		lua_tinker::class_def<IStringA>(L,"Delete",&IStringA::Delete);
		lua_tinker::class_def<IStringA>(L,"ReplaceChar",&IStringA::ReplaceChar);
		lua_tinker::class_def<IStringA>(L,"Replace",&IStringA::Replace);
		lua_tinker::class_def<IStringA>(L,"Remove",&IStringA::Remove);

		lua_tinker::class_def<IStringA>(L,"FindChar",&IStringA::FindChar);
		lua_tinker::class_def<IStringA>(L,"ReverseFind",&IStringA::ReverseFind);

		lua_tinker::class_def<IStringA>(L,"Find",&IStringA::Find);

		lua_tinker::class_def<IStringA>(L,"GetBuffer",&IStringA::GetBuffer);
		lua_tinker::class_def<IStringA>(L,"ReleaseBuffer",&IStringA::ReleaseBuffer);
		lua_tinker::class_def<IStringA>(L,"GetBufferSetLength",&IStringA::GetBufferSetLength);
		lua_tinker::class_def<IStringA>(L,"SetLength",&IStringA::SetLength);
		lua_tinker::class_def<IStringA>(L,"Copy",&IStringA::Copy);
		lua_tinker::class_def<IStringA>(L,"Assign",&IStringA::Assign);
		lua_tinker::class_def<IStringA>(L,"Assign2",&IStringA::Assign2);
		lua_tinker::class_def<IStringA>(L,"GetPrivData",&IStringA::GetPrivData);

		lua_tinker::class_def<IStringA>(L,"ToUpper",&IStringA::ToUpper);
		lua_tinker::class_def<IStringA>(L,"ToLower",&IStringA::ToLower);

		lua_tinker::class_def<IStringA>(L,"TrimRight",&IStringA::TrimRight);
		lua_tinker::class_def<IStringA>(L,"TrimLeft",&IStringA::TrimLeft);
		lua_tinker::class_def<IStringA>(L,"Trim",&IStringA::Trim);
		lua_tinker::class_def<IStringA>(L,"AppendChar",&IStringA::AppendChar);
		lua_tinker::class_def<IStringA>(L,"AppendStr",&IStringA::AppendStr);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}

}

BOOL ExpLua_IStringW(lua_State *L)
{
	try{

		lua_tinker::class_add<IStringW>(L,"IStringW");
		lua_tinker::class_inh<IStringW,IObjRef>(L);

		lua_tinker::class_def<IStringW>(L,"GetLength",&IStringW::GetLength);
		lua_tinker::class_def<IStringW>(L,"IsEmpty",&IStringW::IsEmpty);
		lua_tinker::class_def<IStringW>(L,"Empty",&IStringW::Empty);
		lua_tinker::class_def<IStringW>(L,"GetAt",&IStringW::GetAt);
		lua_tinker::class_def<IStringW>(L,"SetAt",&IStringW::SetAt);

		lua_tinker::class_def<IStringW>(L,"c_str",&IStringW::c_str);
		lua_tinker::class_def<IStringW>(L,"Compare",&IStringW::Compare);
		lua_tinker::class_def<IStringW>(L,"CompareNoCase",&IStringW::CompareNoCase);

		lua_tinker::class_def<IStringW>(L,"TrimBlank",&IStringW::TrimBlank);
		lua_tinker::class_def<IStringW>(L,"InsertChar",&IStringW::InsertChar);
		lua_tinker::class_def<IStringW>(L,"Insert",&IStringW::Insert);
		lua_tinker::class_def<IStringW>(L,"Delete",&IStringW::Delete);
		lua_tinker::class_def<IStringW>(L,"ReplaceChar",&IStringW::ReplaceChar);
		lua_tinker::class_def<IStringW>(L,"Replace",&IStringW::Replace);
		lua_tinker::class_def<IStringW>(L,"Remove",&IStringW::Remove);

		lua_tinker::class_def<IStringW>(L,"FindChar",&IStringW::FindChar);
		lua_tinker::class_def<IStringW>(L,"ReverseFind",&IStringW::ReverseFind);

		lua_tinker::class_def<IStringW>(L,"Find",&IStringW::Find);

		lua_tinker::class_def<IStringW>(L,"GetBuffer",&IStringW::GetBuffer);
		lua_tinker::class_def<IStringW>(L,"ReleaseBuffer",&IStringW::ReleaseBuffer);
		lua_tinker::class_def<IStringW>(L,"GetBufferSetLength",&IStringW::GetBufferSetLength);
		lua_tinker::class_def<IStringW>(L,"SetLength",&IStringW::SetLength);
		lua_tinker::class_def<IStringW>(L,"Copy",&IStringW::Copy);
		lua_tinker::class_def<IStringW>(L,"Assign",&IStringW::Assign);
		lua_tinker::class_def<IStringW>(L,"Assign2",&IStringW::Assign2);
		lua_tinker::class_def<IStringW>(L,"GetPrivData",&IStringW::GetPrivData);

		lua_tinker::class_def<IStringW>(L,"ToUpper",&IStringW::ToUpper);
		lua_tinker::class_def<IStringW>(L,"ToLower",&IStringW::ToLower);

		lua_tinker::class_def<IStringW>(L,"TrimRight",&IStringW::TrimRight);
		lua_tinker::class_def<IStringW>(L,"TrimLeft",&IStringW::TrimLeft);
		lua_tinker::class_def<IStringW>(L,"Trim",&IStringW::Trim);
		lua_tinker::class_def<IStringW>(L,"AppendChar",&IStringW::AppendChar);
		lua_tinker::class_def<IStringW>(L,"AppendStr",&IStringW::AppendStr);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}

}