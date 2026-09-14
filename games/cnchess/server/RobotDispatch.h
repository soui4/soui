#ifndef __ROBOTDISPATCH_H__
#define __ROBOTDISPATCH_H__

#include <ChessLayout.h>

// Robot thinking task. On a pool thread we only run search against a snapshot
// of the board; we never touch live game state here. The result is posted back
// to the game main thread (LWS thread) through the service queue where the move
// is applied serially.
struct SRobotTask
{
    int tableId;       // game table id (used to locate the table when result comes back)
    int seatId;        // robot seat
    int depth;         // search depth converted from intelligence level
    int generation;    // move-message counter at dispatch time, to drop stale results
    CChessLayout layout; // board snapshot (deep copied via Layout::Copy)
};

typedef bool (*PfnRobotDispatch)(const SRobotTask &task);

// Dispatch hook: wired at runtime by the server to the robot AI thread pool
// (CRobotAIPool). When NULL the game table falls back to synchronous search,
// which keeps unit tests decoupled from the thread pool.
extern PfnRobotDispatch g_pfnRobotDispatch;

#endif // !__ROBOTDISPATCH_H__