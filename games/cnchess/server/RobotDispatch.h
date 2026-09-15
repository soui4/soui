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
    int timeMs;        // thinking time budget in ms (0 = fixed depth only)
    CChessLayout layout; // board snapshot (deep copied via Layout::Copy)
};

#endif // !__ROBOTDISPATCH_H__