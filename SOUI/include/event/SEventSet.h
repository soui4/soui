#ifndef __SEVENTSET__H__
#define __SEVENTSET__H__

#include "SEventSlot.h"

SNSBEGIN

/**
 * @class SEvent
 * @brief 表示一个事件对象
 */
class SOUI_EXP SEvent {
  public:
    /**
     * @brief 构造函数
     * @param dwEventID 事件ID
     * @param pszEventName 事件名称
     */
    SEvent(DWORD dwEventID, LPCWSTR pszEventName);

    /**
     * @brief 析构函数
     */
    virtual ~SEvent();

    /**
     * @brief 获取事件ID
     * @return 事件ID
     */
    DWORD GetID();

    /**
     * @brief 获取事件名称
     * @return 事件名称
     */
    SStringW GetName() const;

    /**
     * @brief 获取脚本处理程序
     * @return 脚本处理程序字符串
     */
    SStringA GetScriptHandler() const;

    /**
     * @brief 设置脚本处理程序
     * @param strScriptHandler 脚本处理程序字符串
     */
    void SetScriptHandler(const SStringA &strScriptHandler);

    /**
     * @brief 订阅事件
     * @param slot 事件槽对象
     * @return 成功返回TRUE，失败返回FALSE
     */
    BOOL subscribe(const IEvtSlot *slot);

    /**
     * @brief 取消订阅事件
     * @param slot 事件槽对象
     * @return 成功返回TRUE，失败返回FALSE
     */
    BOOL unsubscribe(const IEvtSlot *slot);

    /**
     * @brief 触发事件
     * @param args 事件参数对象
     */
    void fire(IEvtArgs *args);

  protected:
    /**
     * @brief 查找事件槽对象
     * @param slot 事件槽对象
     * @return 找到的事件槽索引，未找到返回-1
     */
    int findSlotFunctor(const IEvtSlot *slot);

    DWORD m_dwEventID;           ///< 事件ID
    SStringW m_strEventName;     ///< 事件名称
    SStringA m_strScriptHandler; ///< 脚本处理程序字符串

    SArray<IEvtSlot *> m_evtSlots; ///< 事件槽数组
};

/**
 * @class SEventSet
 * @brief 表示一组事件对象
 */
class SOUI_EXP SEventSet {
    friend class SWindow;

  public:
    /**
     * @brief 构造函数
     */
    SEventSet(void);

    /**
     * @brief 析构函数
     */
    virtual ~SEventSet(void);

    /**
     * @brief 添加一个新事件到事件集
     * @param dwEventID 事件ID
     * @param pszEventHandlerName 事件处理程序名称
     * @return 成功返回TRUE，失败返回FALSE
     */
    BOOL addEvent(DWORD dwEventID, LPCWSTR pszEventHandlerName);

    /**
     * @brief 移除指定ID的事件
     * @param dwEventID 事件ID
     * @return 成功返回TRUE，失败返回FALSE
     */
    BOOL removeEvent(DWORD dwEventID);

    /**
     * @brief 移除所有事件对象
     */
    void removeAllEvents(void);

    /**
     * @brief 检查事件集是否包含指定ID的事件
     * @param dwEventID 事件ID
     * @return 存在返回TRUE，不存在返回FALSE
     */
    BOOL isEventPresent(DWORD dwEventID);

    /**
     * @brief 设置事件的脚本处理程序
     * @param strEventName 事件名称
     * @param strScriptHandler 脚本处理程序字符串
     * @return 成功返回TRUE，失败返回FALSE
     */
    BOOL setEventScriptHandler(const SStringW &strEventName, const SStringA strScriptHandler);

    /**
     * @brief 获取事件的脚本处理程序
     * @param strEventName 事件名称
     * @return 脚本处理程序字符串
     */
    SStringA getEventScriptHandler(const SStringW &strEventName) const;

    /**
     * @brief 订阅事件
     * @param dwEventID 事件ID
     * @param subscriber 事件槽对象
     * @return 成功返回TRUE，失败返回FALSE
     */
    BOOL subscribeEvent(DWORD dwEventID, const IEvtSlot &subscriber)
    {
        return subscribeEvent(dwEventID, &subscriber);
    }

    /**
     * @brief 订阅事件
     * @param dwEventID 事件ID
     * @param subscriber 事件槽对象指针
     * @return 成功返回TRUE，失败返回FALSE
     */
    BOOL subscribeEvent(DWORD dwEventID, const IEvtSlot *subscriber);

#if _MSC_VER >= 1700 // VS2012
    /**
     * @brief 订阅事件（使用标准函数回调）
     * @param dwEventID 事件ID
     * @param eventCallback 标准函数回调
     * @return 成功返回TRUE，失败返回FALSE
     */
    BOOL subscribeEvent(DWORD dwEventID, const StdFunCallback &eventCallback);
#endif

    /**
     * @brief 订阅事件（模板函数）
     * @tparam T 对象类型
     * @tparam A 事件参数类型
     * @param pFn 成员函数指针
     * @param pObject 对象指针
     * @return 成功返回TRUE，失败返回FALSE
     */
    template <typename T, typename A>
    BOOL subscribeEvent(BOOL (T::*pFn)(A *), T *pObject)
    {
        return subscribeEvent(A::EventID, Subscriber(pFn, pObject));
    }

    /**
     * @brief 订阅事件（模板函数）
     * @tparam A 事件参数类型
     * @param pFn 函数指针
     * @return 成功返回TRUE，失败返回FALSE
     */
    template <typename A>
    BOOL subscribeEvent(BOOL (*pFn)(A *))
    {
        return subscribeEvent(A::EventID, Subscriber(pFn));
    }

    /**
     * @brief 订阅事件（模板函数）
     * @tparam T 对象类型
     * @param pFn 成员函数指针
     * @param pObject 对象指针
     * @return 成功返回TRUE，失败返回FALSE
     */
    template <typename T>
    BOOL subscribeEvent(DWORD dwEventID, BOOL (T::*pFn)(IEvtArgs *), T *pObject)
    {
        return subscribeEvent(dwEventID, Subscriber(pFn, this));
    }

    /**
     * @brief 取消订阅事件
     * @param dwEventID 事件ID
     * @param subscriber 事件槽对象指针
     * @return 成功返回TRUE，失败返回FALSE
     */
    BOOL unsubscribeEvent(DWORD dwEventID, const IEvtSlot *subscriber);

    /**
     * @brief 取消订阅事件
     * @param dwEventID 事件ID
     * @param subscriber 事件槽对象
     * @return 成功返回TRUE，失败返回FALSE
     */
    BOOL unsubscribeEvent(DWORD dwEventID, const IEvtSlot &subscriber)
    {
        return unsubscribeEvent(dwEventID, &subscriber);
    }

    /**
     * @brief 取消订阅事件（模板函数）
     * @tparam T 对象类型
     * @tparam A 事件参数类型
     * @param pFn 成员函数指针
     * @param pObject 对象指针
     * @return 成功返回TRUE，失败返回FALSE
     */
    template <typename T, typename A>
    BOOL unsubscribeEvent(BOOL (T::*pFn)(A *), T *pObject)
    {
        return unsubscribeEvent(A::EventID, Subscriber(pFn, pObject));
    }

    /**
     * @brief 取消订阅事件（模板函数）
     * @tparam A 事件参数类型
     * @param pFn 函数指针
     * @return 成功返回TRUE，失败返回FALSE
     */
    template <typename A>
    BOOL unsubscribeEvent(BOOL (*pFn)(A *))
    {
        return unsubscribeEvent(A::EventID, Subscriber(pFn));
    }

    /**
     * @brief 取消订阅事件（模板函数）
     * @tparam T 对象类型
     * @param pFn 成员函数指针
     * @param pObject 对象指针
     * @return 成功返回TRUE，失败返回FALSE
     */
    template <typename T>
    BOOL unsubscribeEvent(DWORD dwEventID, BOOL (T::*pFn)(IEvtArgs *), T *pObject)
    {
        return unsubscribeEvent(dwEventID, Subscriber(pFn, this));
    }

    /**
     * @brief 触发事件
     * @param args 事件参数对象
     */
    void FireEvent(IEvtArgs *args);

    /**
     * @brief 检查事件集是否被静音
     * @return 被静音返回TRUE，否则返回FALSE
     */
    BOOL isMuted(void) const
    {
        return m_nMuted > 0;
    }

    /**
     * @brief 设置事件集的静音状态
     * @param setting TRUE表示静音，FALSE表示取消静音
     */
    void setMutedState(BOOL setting);

  protected:
    /**
     * @brief 获取事件对象
     * @param dwEventID 事件ID
     * @return 事件对象指针，未找到返回NULL
     */
    SEvent *GetEventObject(const DWORD dwEventID);

    SArray<SEvent *> m_evtArr; ///< 事件数组
    int m_nMuted;              ///< 静音状态计数
};

SNSEND

#endif // __SEVENTSET__H__