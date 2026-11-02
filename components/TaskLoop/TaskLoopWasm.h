#ifndef __TASKLOOP_WASM_H__
#define __TASKLOOP_WASM_H__

#include <interface/STaskLoop-i.h>

SNSBEGIN

/**
 * @brief 驱动浏览器里的所有 ITaskLoop（宿主主循环每帧调用一次）
 *
 * wasm 版本没有线程：postTask 只是入队，真正的执行发生在宿主主循环里。
 * 每次调用按 budgetMs 时间预算把队列跑一段，没跑完的留到下一帧，
 * 一个慢任务最多拖长一帧，不会把页面卡死。
 *
 * @param budgetMs 本次最多占用多少毫秒；<=0 表示不限制
 * @return 还有多少任务没有执行（0 表示全部跑完）
 */
int STaskLoopPump(double budgetMs);

/** 累计执行过的任务数（烟测用来断言任务确实被泵跑过） */
long STaskLoopRunCount();

/** 当前所有存活实例排队中的任务总数 */
int STaskLoopPendingCount();

/** 存活实例数（析构后应回到 0） */
int STaskLoopLiveCount();

SNSEND

#endif // __TASKLOOP_WASM_H__
