#ifndef WS_LISTENER_H
#define WS_LISTENER_H

#define _WINSOCKAPI_
#include <interface/obj-ref-i.h>
#include <interface/SRunnable-i.h>
#include <stdint.h>

SNSBEGIN

/**
 * @brief WebSocket related constants
 */
enum
{
    kAllGroupId = -1,      /**< Represents all group IDs */
    kDefGroupId = 0,       /**< Default group ID */
    kSocketBufSize = 4096, /**< Socket buffer size */
};

#undef INTERFACE
#define INTERFACE IConnection
/**
 * @brief WebSocket client connection interface
 *
 * Defines the basic operations of WebSocket client connection, including checking connection status,
 * sending text data and binary data. This interface is the core of WebSocket client functionality,
 * providing basic methods for communicating with the server.
 */
DECLARE_INTERFACE_(IConnection, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Check if the connection is valid
     * @return Non-zero indicates valid connection, 0 indicates invalid connection
     */
    STDMETHOD_(int, isValid)(CTHIS) SCONST PURE;

    /**
     * @brief Send text data
     * @param text Text data to send
     * @param nLen Data length, -1 means automatically calculate
     * @return Number of bytes sent on success, -1 on failure
     */
    STDMETHOD_(int, sendText)(THIS_ const char *text, int nLen DEF_VAL(-1)) PURE;

    /**
     * @brief Send binary data
     * @param data Binary data to send
     * @param nLen Data length
     * @return Number of bytes sent on success, -1 on failure
     */
    STDMETHOD_(int, sendBinary)(THIS_ const void *data, int nLen) PURE;
};

#undef INTERFACE
#define INTERFACE ISvrConnection
/**
 * @brief WebSocket server connection interface
 *
 * Inherits from IConnection interface and extends with server-specific connection management functionality.
 * In addition to basic connection operations, it provides type-specific binary data sending, connection closing,
 * connection ID and group ID management, etc., for server-side client connection management.
 */
DECLARE_INTERFACE_(ISvrConnection, IConnection)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Check if the connection is valid
     * @return Non-zero indicates valid connection, 0 indicates invalid connection
     */
    STDMETHOD_(int, isValid)(CTHIS) SCONST PURE;

    /**
     * @brief Send text data
     * @param text Text data to send
     * @param nLen Data length, -1 means automatically calculate
     * @return Number of bytes sent on success, -1 on failure
     */
    STDMETHOD_(int, sendText)(THIS_ const char *text, int nLen DEF_VAL(-1)) PURE;

    /**
     * @brief Send binary data
     * @param data Binary data to send
     * @param nLen Data length
     * @return Number of bytes sent on success, -1 on failure
     */
    STDMETHOD_(int, sendBinary)(THIS_ const void *data, int nLen) PURE;

    /**
     * @brief Send typed binary data
     * @param dwType Data type
     * @param data Binary data to send
     * @param nLen Data length
     * @return Number of bytes sent on success, -1 on failure
     */
    STDMETHOD_(int, sendBinary2)(THIS_ DWORD dwType, const void *data, int nLen) PURE;

    /**
     * @brief Close connection
     * @param reason Close reason
     */
    STDMETHOD_(void, close)(THIS_ const char *reason) PURE;

    /**
     * @brief Set connection ID
     * @param id Connection ID
     */
    STDMETHOD_(void, setId)(THIS_ int id) PURE;

    /**
     * @brief Get connection ID
     * @return Connection ID
     */
    STDMETHOD_(int, getId)(CTHIS) SCONST PURE;

    /**
     * @brief Set connection group ID
     * @param id Group ID
     */
    STDMETHOD_(void, setGroupId)(THIS_ int id) PURE;

    /**
     * @brief Get connection group ID
     * @return Group ID
     */
    STDMETHOD_(int, getGroupId)(CTHIS) SCONST PURE;
};

#undef INTERFACE
#define INTERFACE IConnListener
/**
 * @brief WebSocket client connection listener interface
 *
 * Defines various event callback methods for WebSocket client connections, used to handle connection establishment,
 * errors, disconnection, data sending completion, and data reception events. By implementing this interface,
 * you can listen to and handle all state changes and data interactions of WebSocket client connections.
 */
DECLARE_INTERFACE_(IConnListener, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Connection established successfully event
     */
    STDMETHOD_(void, onConnected)(THIS) PURE;

    /**
     * @brief Connection error event
     * @param errStr Error information
     */
    STDMETHOD_(void, onConnError)(THIS_ const char *errStr) PURE;

    /**
     * @brief Connection disconnected event
     */
    STDMETHOD_(void, onDisconnect)(THIS) PURE;

    /**
     * @brief Data sending completed event
     * @param nMsgId Message ID
     */
    STDMETHOD_(void, onDataSent)(THIS_ int nMsgId) PURE;

    /**
     * @brief Data received event
     * @param data Received data
     * @param len Data length
     * @param bBinary Whether it is binary data, TRUE for binary, FALSE for text
     */
    STDMETHOD_(void, onDataRecv)(THIS_ const void *data, int len, BOOL bBinary) PURE;
};

#undef INTERFACE
#define INTERFACE ISvrListener
/**
 * @brief WebSocket server connection listener interface
 *
 * Defines various event callback methods for WebSocket server side, used to handle client connection establishment,
 * errors, disconnection, data sending completion, and data reception events. By implementing this interface,
 * you can listen to and handle all state changes and data interactions of client connections. It is a core component
 * of server-side WebSocket applications.
 */
DECLARE_INTERFACE_(ISvrListener, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Client connection established successfully event
     * @param pConn Client connection object
     * @param uriPath Requested URI path
     * @param uriArgs Requested URI arguments
     * @return TRUE to accept connection, FALSE to reject connection
     */
    STDMETHOD_(BOOL, onConnected)(THIS_ ISvrConnection * pConn, const char *uriPath, const char *uriArgs) PURE;

    /**
     * @brief Client connection error event
     * @param pConn Client connection object
     * @param errStr Error information
     */
    STDMETHOD_(void, onConnError)(THIS_ ISvrConnection * pConn, const char *errStr) PURE;

    /**
     * @brief Client connection disconnected event
     * @param pConn Client connection object
     */
    STDMETHOD_(void, onDisconnect)(THIS_ ISvrConnection * pConn) PURE;

    /**
     * @brief Client data sending completed event
     * @param pConn Client connection object
     * @param nMsgId Message ID
     */
    STDMETHOD_(void, onDataSent)(THIS_ ISvrConnection * pConn, int nMsgId) PURE;

    /**
     * @brief Client data received event
     * @param pConn Client connection object
     * @param data Received data
     * @param len Data length
     * @param bBinary Whether it is binary data, TRUE for binary, FALSE for text
     */
    STDMETHOD_(void, onDataRecv)(THIS_ ISvrConnection * pConn, const void *data, int len, BOOL bBinary) PURE;
};

/**
 * @brief WebSocket server option structure
 */
typedef struct _SvrOption
{
    BOOL bSecure;            /**< Whether to use secure connection */
    const char *cert_u8;     /**< Certificate data for secure connection */
    const char *priv_key_u8; /**< Private key data for secure connection */
} SvrOption;

/**
 * @brief WebSocket server heartbeat configuration structure
 */
typedef struct _SvrPingCfg
{
    uint32_t pingIntervalSeconds; /**< Ping sending interval in seconds. The server periodically sends pings to check if clients are online */
    uint32_t nHeartbeatSeconds;   /**< Heartbeat timeout in seconds. If the client has no activity (sending data or pong) within this time, the connection will be closed */
    uint32_t nPingTimeoutCount;   /**< Maximum number of ping timeouts. If the number of consecutive pings sent without receiving pongs exceeds this value, the connection will be closed */
                                  /** Note: nHeartbeatSeconds should be greater than pingIntervalSeconds * nPingTimeoutCount, otherwise heartbeat timeout will trigger first */
} SvrPingCfg;

#undef INTERFACE
#define INTERFACE ITimerListener
/**
 * @brief Timer expiration listener interface
 *
 * Implemented by the business layer: it is the response to a timer produced by ITimerGenerator.
 * The generator knows nothing about what a timer means, it only reports the fired ID, so the same
 * mechanism serves every periodic or one-shot job on the server side.
 *
 * This interface carries no reference counting: the caller keeps ownership of its implementation
 * and must keep it alive while it is installed on a generator (i.e. between start() and stop()).
 */
DECLARE_INTERFACE(ITimerListener)
{
    /**
     * @brief A timer expired
     * @param uTimerID Timer ID, as registered by ITimerGenerator::setTimer
     * @remark Invoked on the generator's dedicated timer thread.
     */
    STDMETHOD_(void, onTimer)(THIS_ UINT_PTR uTimerID) PURE;
};

#undef INTERFACE
#define INTERFACE ITimerGenerator
/**
 * @brief Timer generator
 *
 * Owns one dedicated timer thread that keeps every timer in an ID-keyed schedule. When a timer
 * becomes due the thread invokes ITimerListener::onTimer directly on that timer thread; it does
 * NOT marshal to the server event thread, so the business layer decides (per timer) whether to
 * switch threads (e.g. via IWsServer::postServiceTask). What a timer actually does is decided by
 * the business layer through its ITimerListener.
 *
 * This is a ref-counted object (it derives IObjRef): obtain it from
 * IWebsocket::CreateTimerGenerator and manage it through an SAutoRefPtr (or AddRef/Release). The
 * generator is independent of any server instance; stop() (or destruction) joins its thread, so
 * let the listener object outlive the generator.
 */
DECLARE_INTERFACE_(ITimerGenerator,IObjRef)
{
    /**
     * @brief Install the listener and start the timer thread
     * @param pListener Business layer implementation; must outlive the generator until stop()
     * @return TRUE if the thread was started, FALSE if it is already running
     * @remark Call start() before or after setTimer(); the first tick of every timer is measured
     *         from the moment the thread starts, so timers armed earlier are re-based to
     *         "now + period" at start. Calling start() again after stop() is allowed: it resets
     *         the stop flag and spawns a fresh thread.
     */
    STDMETHOD_(BOOL, start)(THIS_ ITimerListener * pListener) PURE;

    /**
     * @brief Stop the timer thread and detach the listener
     * @remark Sets the stop flag, clears the listener, and joins the timer thread. Pending ticks
     *         already collected but not yet dispatched are dropped. Safe to call from any thread
     *         except the timer thread itself.
     */
    STDMETHOD_(void, stop)(THIS) PURE;

    /**
     * @brief Create, update or remove a timer
     * @param uTimerID Timer ID used to identify the timer; 0 asks the generator to allocate a free one
     * @param uIntervalMs Timer period in milliseconds; 0 removes the timer identified by uTimerID
     * @param bRepeat TRUE (default) for a repeating timer that fires every uIntervalMs until it is
     *                killed or the generator is stopped; FALSE for a one-shot timer that fires
     *                exactly once and is then removed automatically.
     * @return The assigned/kept timer ID (non-zero) on create or update; 0 on removal (even if
     *         successful), when the generator is stopped, or when an auto-ID cannot be allocated.
     * @remark Reusing an existing ID replaces both its period and its repeat flag, and re-bases its
     *         next tick to "now + period". May be called before or after start().
     */
    STDMETHOD_(UINT_PTR, setTimer)(THIS_ UINT_PTR uTimerID, uint32_t uIntervalMs, BOOL bRepeat DEF_VAL(TRUE)) PURE;

    /**
     * @brief Remove a timer
     * @param uTimerID Timer ID returned by setTimer
     * @return TRUE if a timer was removed, FALSE if there is no such timer
     */
    STDMETHOD_(BOOL, killTimer)(THIS_ UINT_PTR uTimerID) PURE;
};

#undef INTERFACE
#define INTERFACE IWsServer
/**
 * @brief WebSocket server interface
 *
 * Defines the core operations of WebSocket server, including starting the server, waiting for events, and exiting the server.
 * This interface is the core of WebSocket server functionality, responsible for managing the server's lifecycle and event processing.
 * Through this interface, you can create and control a complete WebSocket server instance.
 */
DECLARE_INTERFACE_(IWsServer, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Start WebSocket server
     * @param port Server listening port
     * @param protocolName Protocol name
     * @param option Server options
     * @param pingCfg Heartbeat configuration
     * @return 0 for success, non-zero for failure
     */
    STDMETHOD_(int, start)(THIS_ uint16_t port, const char *protocolName, SvrOption option, SvrPingCfg pingCfg) PURE;

    /**
     * @brief Wait for server events
     * @param timeout Timeout in milliseconds
     * @return TRUE for event processed, FALSE for timeout
     */
    STDMETHOD_(BOOL, wait)(THIS_ int timeout) PURE;

    /**
     * @brief Quit server
     */
    STDMETHOD_(void, quit)(THIS) PURE;

    /**
     * @brief Post a task to the server event thread (LWS thread) for execution
     * @param task Runnable task object to be executed on the server event thread
     * @remark Intended for other threads (such as AI worker threads) to safely hand results back and serialize callbacks onto the main game thread
     */
    STDMETHOD_(void, postServiceTask)(THIS_ const IRunnable * task) PURE;
};

/**
 * @brief WebSocket client option structure
 */
typedef struct _ClientOption
{
    BOOL bSecure;                     /**< Whether to use secure connection */
    const char *ca_u8;                /**< CA certificate data for secure connection */
    BOOL allowSelfSigned;             /**< Whether to allow self-signed certificates */
    BOOL skipServerCertHostnameCheck; /**< Whether to skip server certificate hostname check */
    BOOL allowExpired;                /**< Whether to allow expired certificates */
} ClientOption;

#undef INTERFACE
#define INTERFACE IWsClient
/**
 * @brief WebSocket client interface
 *
 * Inherits from IConnection interface and extends with WebSocket client-specific functionality.
 * In addition to basic connection operations, it provides connecting to server, disconnecting, waiting for events,
 * and setting receive blocking mode. This interface is the core of WebSocket client applications,
 * responsible for managing the client's lifecycle and communication with the server.
 */
DECLARE_INTERFACE_(IWsClient, IConnection)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Check if connection is valid
     * @return Non-zero indicates valid connection, 0 indicates invalid connection
     */
    STDMETHOD_(int, isValid)(CTHIS) SCONST PURE;

    /**
     * @brief Send text data
     * @param text Text data to send
     * @param nLen Data length, -1 means automatically calculate
     * @return Number of bytes sent on success, -1 on failure
     */
    STDMETHOD_(int, sendText)(THIS_ const char *text, int nLen DEF_VAL(-1)) PURE;

    /**
     * @brief Send binary data
     * @param data Binary data to send
     * @param nLen Data length
     * @return Number of bytes sent on success, -1 on failure
     */
    STDMETHOD_(int, sendBinary)(THIS_ const void *data, int nLen) PURE;

    /**
     * @brief Send typed binary data
     * @param dwType Data type
     * @param data Binary data to send
     * @param nLen Data length
     * @return Number of bytes sent on success, -1 on failure
     */
    STDMETHOD_(int, sendBinary2)(THIS_ DWORD dwType, const void *data, int nLen) PURE;

    /**
     * @brief Connect to WebSocket server
     * @param pszSvr Server address
     * @param pszPath Request path
     * @param nPort Server port
     * @param pszProtocol Protocol name
     * @param option Client options
     * @return 0 for success, non-zero for failure
     */
    STDMETHOD_(int, connectTo)(THIS_ const char *pszSvr, const char *pszPath, uint16_t nPort, const char *pszProtocol, ClientOption option) PURE;

    /**
     * @brief Disconnect from server
     */
    STDMETHOD_(void, disconnect)(THIS) PURE;

    /**
     * @brief Wait for client events
     * @param timeoutMs Timeout in milliseconds
     * @return TRUE for event processed, FALSE for timeout
     */
    STDMETHOD_(BOOL, wait)(THIS_ int timeoutMs) PURE;

    /**
     * @brief Set receive blocking mode
     * @param bBlock TRUE for blocking mode, FALSE for non-blocking mode
     */
    STDMETHOD_(void, blockReceive)(THIS_ BOOL bBlock) PURE;
};

/**
 * @brief WebSocket log callback function type
 * @param level Log level
 * @param line Log content
 */
typedef void (*WsLogCallback)(int level, const char *line);

#undef INTERFACE
#define INTERFACE IWebsocket
/**
 * @brief WebSocket core interface
 *
 * Defines the core functionality of the WebSocket module, including creating WebSocket client and server instances,
 * and setting log callback functions. This interface is the entry point of the WebSocket module, through which
 * WebSocket client and server instances can be created and managed. It is the foundation of the entire WebSocket functionality.
 */
DECLARE_INTERFACE_(IWebsocket, IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Create WebSocket client instance
     * @param pListener Client event listener
     * @return WebSocket client instance pointer
     */
    STDMETHOD_(IWsClient *, CreateWsClient)(THIS_ IConnListener * pListener) PURE;

    /**
     * @brief Create WebSocket server instance
     * @param pListener Server event listener
     * @return WebSocket server instance pointer
     */
    STDMETHOD_(IWsServer *, CreateWsServer)(THIS_ ISvrListener * pListener) PURE;

    /**
     * @brief Create Timer generator
     */
    STDMETHOD_(ITimerGenerator *,CreateTimerGenerator)(CTHIS) PURE;

    /**
     * @brief Set log callback function
     * @param cb Log callback function pointer
     */
    STDMETHOD_(void, SetLogCallback)(THIS_ WsLogCallback cb) PURE;
};

SNSEND
#endif /**< WS_LISTENER_H */