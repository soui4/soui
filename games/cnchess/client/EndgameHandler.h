#ifndef ENDGAMEHANDLER_H
#define ENDGAMEHANDLER_H

#include <sobject/Sobject.hpp>
#include <helper/obj-ref-impl.hpp>
#include <vector>
#include "WebSocketClient.h"
#include "ConnListener-i.h"
#include "SGameTheme.h"
#include "ChessBoard.h"
#include "ChessPiece.h"
#include "protocol.h"
class CEndgameAdapter;
class CMainDlg;

/**
 * @brief 残局打谱处理器
 *
 * 采用与普通大厅一致的"游戏桌"模型: 每个残局按配置占用 N 张游戏桌
 * (桌号 = ENDGAME_TABLE_BASE + 残局序号*N + 槽位, N 由服务端 tablesPerEndgame 下发), 坐哪桌就对应哪一个残局。
 * 本处理器负责展示残局桌列表、坐桌、邀请机器人, 并在右侧/底部棋盘上预览
 * 当前选中残局的布局。
 */
class EndgameHandler : public TObjRefImpl<SObject>, public IConnListener2
{
    DEF_SOBJECT(SObject, L"endgamehandler")
    friend class CEndgameAdapter;
public:
    EndgameHandler(CMainDlg *pMainDlg, SGameTheme *pTheme);
    virtual ~EndgameHandler();

    /**
     * @brief 初始化处理器
     * @param pRoot 残局页根窗口
     * @param pWs WebSocket客户端
     */
    void Init(SWindow *pRoot);
    void SetWebSocket(WebSocketClient* pWs);
    void OnThemdReady();
public:
    EVENT_MAP_BEGIN()
        EVENT_CHECK_SENDER_ROOT(m_pRoot)
        EVENT_NAME_HANDLER(L"chessboard_preview", EventSwndSize::EventID, OnPreviewSizeChanged)
    EVENT_MAP_BREAK()

public:
    void OnConnected() override;
    void OnDisconnected() override;
    BOOL OnMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE> > data);

    // 请求残局列表
    void RequestList();
    // 请求坐下(残局桌号 = BASE + 残局序号*2 + 0/1)
    void ReqSeatDown(int iTable, int iSeat);
    // 邀请机器人入座
    void ReqRobotInvite(int iTable, int iSeat, int nLevel);
    // 选中残局并更新右侧预览
    void SelectEndgame(int nEndgameIndex);
    // 提示
    void PlayTip(LPCTSTR pszTip);

    // 棋子锚点定位回调
    static POINT CALLBACK ChessAnchor2Pos(const AnchorPos &pos, const CRect &rcParent,
                                          const CSize &szChild, int nScale, void *userData);
protected:
    BOOL OnEndgameListAck(const void *lpData, int nSize);
    BOOL OnRoomInfo(const void *lpData, int nSize);
    BOOL OnTableInfo(const void *lpData, int nSize);
    BOOL OnSeatDownAck(const void *lpData, int nSize);
    BOOL OnRobotInviteAck(const void *lpData, int nSize);
    void OnPreviewSizeChanged(IEvtArgs *e);
    void ClearPieces();
    void RenderLayout(const int layout[10][9]);
    // 残局桌坐满后自动就绪并跳转到对局页
    void TryAutoStart(GAME_TABLE_INFO *pInfo);
private:
    CMainDlg *m_pMainDlg;
    SGameTheme *m_pTheme;
    SWindow *m_pRoot;
    WebSocketClient *m_ws;
    CEndgameAdapter *m_pAdapter;
    CChessBoard *m_pPreviewBoard;
    SImageWnd *m_pChessBoard;
    std::vector<IWindow *> m_pieces;      ///< 预览棋子窗口
    float m_cellWidth;
    float m_cellHeight;
    CPoint m_ptBoardOrigin;               ///< 棋盘左下角坐标
    int m_nCurEndgameIndex;               ///< 当前选中的残局序号
    int m_nTablesPerEndgame;              ///< 每残局桌数(由服务端配置下发, 默认2)
    bool m_bAutoStartSent;                ///< 是否已为本局自动发送过就绪
};

#endif//ENDGAMEHANDLER_H