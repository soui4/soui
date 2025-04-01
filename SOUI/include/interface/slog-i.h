#ifndef __SLOG_I__H__
#define __SLOG_I__H__

#include <interface/obj-ref-i.h>
#include <windows.h>
#include <time.h>

SNSBEGIN

//! the max log content length.
enum
{
    LOG4Z_LOG_BUF_SIZE = 10240,
};

//! LOG Level
enum ENUM_LOG_LEVEL
{
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_ALARM,
    LOG_LEVEL_FATAL,
};

#undef INTERFACE
#define INTERFACE IOutputFileBuilder
DECLARE_INTERFACE_(IOutputFileBuilder, IObjRef)
{
    //! 添加引用
    /*!
     * @return long - 当前引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //! 释放引用
    /*!
     * @return long - 当前引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //! 释放对象
    /*!
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    //! 生成LOG文件名
    /*!
     * @param pszFileName - 输出文件名缓冲区
     * @param nLen - 缓冲区长度
     * @param pszLogName - 日志名称
     * @param pid - 进程ID
     * @param curFileIndex - 当前文件索引
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, buildOutputFile)
    (CTHIS_ char *pszFileName, int nLen, const char *pszLogName, unsigned long pid, int curFileIndex) SCONST PURE;
};

//! log4z class
#undef INTERFACE
#define INTERFACE ILogMgr
DECLARE_INTERFACE_(ILogMgr, IObjRef)
{
    //! 添加引用
    /*!
     * @return long - 当前引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //! 释放引用
    /*!
     * @return long - 当前引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //! 释放对象
    /*!
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    //! 配置或覆盖配置
    /*!
     * @param configPath - 配置文件路径
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, config)(THIS_ const char *configPath) PURE;

    //! 从字符串配置
    /*!
     * @param configContent - 配置内容字符串
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, configFromString)(THIS_ const char *configContent) PURE;

    //! 启动日志线程
    /*!
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, start)(THIS) PURE;

    //! 停止日志线程
    /*!
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, stop)(THIS) PURE;

    //! 预检查日志过滤器
    /*!
     * @param level - 日志级别
     * @return BOOL - 过滤通过返回TRUE，过滤失败返回FALSE
     */
    STDMETHOD_(BOOL, prePushLog)(THIS_ int level) PURE;

    //! 推送日志，线程安全
    /*!
     * @param level - 日志级别
     * @param filter - 日志过滤器
     * @param log - 日志内容
     * @param file - 文件名
     * @param line - 行号
     * @param func - 函数名
     * @param pRetAddr - 返回地址
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, pushLog)
    (THIS_ int level, const char *filter, const char *log, const char *file, int line, const char *func, const void *pRetAddr) PURE;

    //! 检查日志器是否启用
    /*!
     * @return BOOL - 启用返回TRUE，未启用返回FALSE
     */
    STDMETHOD_(BOOL, isLoggerEnable)(CTHIS) SCONST PURE;

    //! 启用或禁用日志器
    /*!
     * @param enable - 启用标志
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, enableLogger)(THIS_ BOOL enable) PURE;

    //! 设置日志器名称
    /*!
     * @param name_u8 - 日志器名称
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, setLoggerName)(THIS_ const char *name_u8) PURE;

    //! 设置日志器路径
    /*!
     * @param path_u8 - 日志器路径
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, setLoggerPath)(THIS_ const char *path_u8) PURE;

    //! 设置日志级别
    /*!
     * @param nLevel - 日志级别
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, setLoggerLevel)(THIS_ int nLevel) PURE;

    //! 启用或禁用文件行号
    /*!
     * @param enable - 启用标志
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, setLoggerFileLine)(THIS_ BOOL enable) PURE;

    //! 启用或禁用日志显示
    /*!
     * @param enable - 启用标志
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, setLoggerDisplay)(THIS_ BOOL enable) PURE;

    //! 启用或禁用日志输出到文件
    /*!
     * @param enable - 启用标志
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, setLoggerOutFile)(THIS_ BOOL enable) PURE;

    //! 设置日志文件大小限制
    /*!
     * @param limitsize - 文件大小限制
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, setLoggerLimitsize)(THIS_ unsigned int limitsize) PURE;

    //! 设置日志输出到文件的规则
    /*!
     * @param pOutputFileBuilder - 输出文件生成器
     * @return void
     */
    STDMETHOD_(void, setOutputFileBuilder)(THIS_ IOutputFileBuilder * pOutputFileBuilder) PURE;
};

SNSEND
#endif // __SLOG_I__H__
