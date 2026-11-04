#ifndef __TASKLOOP_WASM_H__
#define __TASKLOOP_WASM_H__

#include <interface/STaskLoop-i.h>

SNSBEGIN

/**
 * @brief 驱动浏览器里的所有 ITaskLoop（宿主主循环每帧调用一次）
 *
 * 这个后端有两条路，按构建能力选定，对上仍是同一套 ITaskLoop 语义：
 *   - 泵后端（不带 -pthread 的构建）：postTask 只是入队，真正的执行发生在
 *     宿主主循环里。每次调用按 budgetMs 时间预算把队列跑一段，没跑完的留到下
 *     一帧，一个慢任务最多拖长一帧，不会把页面卡死。
 *   - worker 后端（带 -pthread 的构建）：start() 建一条 wasm worker 线程，队列
 *     归它消费，UI 线程只入队 + 唤醒。这类实例会被本函数跳过 —— 再跑一遍就是
 *     重复执行；慢任务因此整条离开主线程，一帧都不该被它拖住。
 *
 * @param budgetMs 本次最多占用多少毫秒；<=0 表示不限制
 * @return 还有多少任务没有执行（0 表示全部跑完）
 */
int STaskLoopPump(double budgetMs);

/** 累计执行过的任务数（烟测用来断言任务确实被跑过） */
long STaskLoopRunCount();

/**
 * 其中由 worker 线程执行的任务数。
 *
 * 这条计数只在 worker 后端递增：泵后端里所有执行都发生在调用 STaskLoopPump 的那条
 * 线程上，永远读到 0。宿主用它来证明"后台任务确实离开了 UI 线程"，而不是靠"任务跑
 * 完了"间接推断 —— 后者两条后端都给得出，区分不了这次改动的目标。
 */
long STaskLoopWorkerRunCount();

/** 当前所有存活实例排队中的任务总数 */
int STaskLoopPendingCount();

/** 存活实例数（析构后应回到 0） */
int STaskLoopLiveCount();

/** 其中由 worker 线程驱动的实例数：为 0 说明退回泵后端（构建没带 -pthread，或线程创建失败） */
int STaskLoopWorkerCount();

/**
 * SCOMMgr::CreateTaskLoop 走的就是这个入口（components/commgr2.h 按 SCOM_MASK 展开）。
 * 桌面版把它声明在 thread/TaskLoop.h 里，而那份头会再拉进 thread/thread.h；wasm 宿主只想要队列，
 * 所以这条声明在本头里自己给出一份，宿主不必为了建实例去 include 桌面头。
 */
namespace TASKLOOP
{
SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef **ppTaskLoop);
}

SNSEND

EXTERN_C BOOL SOUI_COM_API TaskLoop_SCreateInstance(IObjRef **ppTaskLoop);

#endif // __TASKLOOP_WASM_H__
