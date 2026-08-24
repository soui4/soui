#pragma once

class CLoginDlg : public SHostDialog, public SDpiHandler<CLoginDlg>
{
public:
    CLoginDlg(); 
public:
    void OnBtnLocal();
    // 选择外部图片文件
    void OnAvatarExt();
    //soui消息
    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_close", OnClose)
        EVENT_NAME_COMMAND(L"btn_login", OnLogin)
        EVENT_NAME_COMMAND(L"btn_svr_local", OnBtnLocal)
        EVENT_NAME_COMMAND_MAGIC(L"btn_avatar_1", OnSelectBuiltinAvatar,1)
        EVENT_NAME_COMMAND_MAGIC(L"btn_avatar_2", OnSelectBuiltinAvatar,2)
        EVENT_NAME_COMMAND_MAGIC(L"btn_avatar_3", OnSelectBuiltinAvatar,3)
        EVENT_NAME_COMMAND_MAGIC(L"btn_avatar_4", OnSelectBuiltinAvatar,4)
        EVENT_NAME_COMMAND(L"btn_avatar_ext", OnAvatarExt)
    EVENT_MAP_END2(SHostDialog)
    
    //HostWnd真实窗口消息处理
    BEGIN_MSG_MAP_EX(CLoginDlg)
        CHAIN_MSG_MAP(SDpiHandler<CLoginDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)
        CHAIN_MSG_MAP(SHostDialog)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

private:
    void OnClose();
    void OnLogin();
    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
    void OnSelectBuiltinAvatar(int nId);
    void UpdateAvatarPreview();    // 根据当前选择更新预览窗口
    void SetAvatarSelectionBorder(int nId); // 设置选中的边框高亮
public:
    SStringT m_strSvr;
    SStringT m_strName;
    char m_cSex;
    int m_nAvatarId;            // 内置头像ID (1..5) 或 -1 (外部)
    SStringT m_strAvatarPath;   // 外部图片完整路径 (本地)
};
