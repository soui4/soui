#ifndef EVENTS_H
#define EVENTS_H

#include <event/SEvents.h>
#include <protocol.h>

#define EVT_GAME_BEGIN (EVT_EXTERNAL_BEGIN + 6000)
enum{
    EVT_TABLE_INFO = EVT_GAME_BEGIN,
};

DEF_EVT_EXT(EventTableInfo, EVT_TABLE_INFO, {
    GAME_TABLE_INFO *pTableInfo;
    int nSize;
})


#endif//EVENTS_H