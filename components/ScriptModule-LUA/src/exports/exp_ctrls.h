#include <control/SouiCtrls.h>
#include "toobj.h"


BOOL ExpLua_Ctrls(lua_State *L)
{
	try{
		lua_tinker::class_add<SComboBase>(L,"SComboBase");
		lua_tinker::class_inh<SComboBase,SWindow>(L);
		lua_tinker::class_def<SComboBase>(L,"GetCurSel",&SComboBase::GetCurSel);
		lua_tinker::class_def<SComboBase>(L,"GetCount",&SComboBase::GetCount);
		lua_tinker::class_def<SComboBase>(L,"SetCurSel",&SComboBase::SetCurSel);
		lua_tinker::class_def<SComboBase>(L,"GetLBText",&SComboBase::GetLBText);
		lua_tinker::class_def<SComboBase>(L,"FindString",&SComboBase::FindString);
		lua_tinker::class_def<SComboBase>(L,"DropDown",&SComboBase::DropDown);
		lua_tinker::class_def<SComboBase>(L,"CloseUp",&SComboBase::CloseUp);
		DEF_TOOBJ(L,SComboBase);
		
        lua_tinker::class_add<SComboBox>(L,"SComboBox");
        lua_tinker::class_inh<SComboBox,SComboBase>(L);
        lua_tinker::class_def<SComboBox>(L,"InsertItem",&SComboBox::InsertItem);
        lua_tinker::class_def<SComboBox>(L,"DeleteString",&SComboBox::DeleteString);
        lua_tinker::class_def<SComboBox>(L,"ResetContent",&SComboBox::ResetContent);
        lua_tinker::class_def<SComboBox>(L,"GetLBText",&SComboBox::GetLBText);
        lua_tinker::class_def<SComboBox>(L,"GetListBox",&SComboBox::GetListBox);
        lua_tinker::class_def<SComboBox>(L,"GetItemData",&SComboBox::GetItemData);
        lua_tinker::class_def<SComboBox>(L,"SetItemData",&SComboBox::SetItemData);
		DEF_TOOBJ(L,SComboBox);

		lua_tinker::class_add<SOsrPanel>(L,"SOsrPanel");
		lua_tinker::class_inh<SOsrPanel,SWindow>(L);
		DEF_TOOBJ(L,SOsrPanel);

		lua_tinker::class_add<SItemPanel>(L,"SItemPanel");
		lua_tinker::class_inh<SItemPanel,SOsrPanel>(L);
		DEF_TOOBJ(L,SItemPanel);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}