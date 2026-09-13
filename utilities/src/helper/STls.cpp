#include <windows.h>
#include <helper/STls.h>

SNSBEGIN

// Number of TLS slots to allocate at once when growing the per-thread array.
#define TLS_ALLOC_CHUNKSIZE 4

// A single TLS slot entry: stored value + optional destructor.
struct STlsEntry
{
    void *data;
    STlsDestructor destructor;
};

// Per-thread TLS storage: a growable array of STlsEntry.
struct STlsStorage
{
    int limit;
    STlsEntry *array;
};

// Generic TLS list node, keyed by thread ID (mirrors SDL3's SDL_TLSEntry).
struct STlsNode
{
    unsigned long long thread_id;
    STlsStorage *storage;
    STlsNode *next;
};

// Global state
static STlsNode *g_tls_list = NULL;
static LONG g_tls_id_counter = 0;

// ---- Platform primitives ----


static CRITICAL_SECTION g_tls_mutex;
static INIT_ONCE g_tls_init_once = INIT_ONCE_STATIC_INIT;

static BOOL CALLBACK TlsInitOnce(PINIT_ONCE /*InitOnce*/, PVOID Parameter, PVOID * /*Context*/)
{
    InitializeCriticalSection((CRITICAL_SECTION *)Parameter);
    return TRUE;
}

static void TlsLock()
{
    InitOnceExecuteOnce(&g_tls_init_once, TlsInitOnce, &g_tls_mutex, NULL);
    EnterCriticalSection(&g_tls_mutex);
}

static void TlsUnlock()
{
    LeaveCriticalSection(&g_tls_mutex);
}

static tid_t TlsGetThreadId()
{
    return GetCurrentThreadId();
}

static LONG TlsAtomicIncrement(LONG volatile *val)
{
    return InterlockedIncrement(val);
}

static LONG TlsAtomicCompareExchange(LONG volatile *dest, LONG exchange, LONG comparand)
{
    return InterlockedCompareExchange(dest, exchange, comparand);
}

static LONG TlsAtomicGet(LONG volatile *val)
{
    return InterlockedExchangeAdd(val, 0);
}

// ---- STls implementation ----

void *STls::Get(STlsId *id)
{
    if (!id)
        return NULL;

    int storage_index = (int)TlsAtomicGet(id) - 1;
    if (storage_index < 0)
        return NULL;

    unsigned long long tid = TlsGetThreadId();
    TlsLock();
    STlsStorage *storage = NULL;
    for (STlsNode *node = g_tls_list; node; node = node->next)
    {
        if (node->thread_id == tid)
        {
            storage = node->storage;
            break;
        }
    }
    TlsUnlock();

    if (!storage || storage_index >= storage->limit)
        return NULL;
    return storage->array[storage_index].data;
}

BOOL STls::Set(STlsId *id, void *value, STlsDestructor destructor)
{
    if (!id)
        return FALSE;

    // Lazily allocate a global slot index for this ID (mirrors SDL3).
    int storage_index = (int)TlsAtomicGet(id) - 1;
    if (storage_index < 0)
    {
        long new_id = TlsAtomicIncrement(&g_tls_id_counter);
        TlsAtomicCompareExchange(id, new_id, 0);
        // If raced, every thread ends up with the same index.
        storage_index = (int)TlsAtomicGet(id) - 1;
    }

    unsigned long long tid = TlsGetThreadId();
    TlsLock();

    // Find this thread's storage node.
    STlsNode *node = NULL;
    for (node = g_tls_list; node; node = node->next)
    {
        if (node->thread_id == tid)
            break;
    }

    STlsStorage *storage = node ? node->storage : NULL;

    // Grow the array if needed.
    if (!storage || storage_index >= storage->limit)
    {
        int oldlimit = storage ? storage->limit : 0;
        int newlimit = storage_index + TLS_ALLOC_CHUNKSIZE;
        STlsEntry *new_array = (STlsEntry *)realloc(storage ? storage->array : NULL,
                                                     (size_t)newlimit * sizeof(STlsEntry));
        if (!new_array)
        {
            TlsUnlock();
            return FALSE;
        }
        for (int i = oldlimit; i < newlimit; ++i)
        {
            new_array[i].data = NULL;
            new_array[i].destructor = NULL;
        }
        if (!storage)
        {
            storage = (STlsStorage *)malloc(sizeof(STlsStorage));
            if (!storage)
            {
                // new_array may be a fresh allocation; free it to avoid leak.
                // (If storage was NULL, storage->array was NULL, so realloc
                // returned a fresh block.)
                free(new_array);
                TlsUnlock();
                return FALSE;
            }
            storage->limit = 0;
        }
        storage->array = new_array;
        storage->limit = newlimit;

        if (!node)
        {
            node = (STlsNode *)malloc(sizeof(STlsNode));
            if (!node)
            {
                free(storage->array);
                free(storage);
                TlsUnlock();
                return FALSE;
            }
            node->thread_id = tid;
            node->storage = storage;
            node->next = g_tls_list;
            g_tls_list = node;
        }
        else
        {
            node->storage = storage;
        }
    }

    storage->array[storage_index].data = value;
    storage->array[storage_index].destructor = destructor;

    TlsUnlock();
    return TRUE;
}

void STls::Cleanup()
{
    unsigned long long tid = TlsGetThreadId();
    TlsLock();

    STlsNode *prev = NULL;
    for (STlsNode *node = g_tls_list; node; node = node->next)
    {
        if (node->thread_id == tid)
        {
            STlsStorage *storage = node->storage;
            if (storage)
            {
                for (int i = 0; i < storage->limit; ++i)
                {
                    if (storage->array[i].destructor)
                        storage->array[i].destructor(storage->array[i].data);
                }
                free(storage->array);
                free(storage);
            }
            if (prev)
                prev->next = node->next;
            else
                g_tls_list = node->next;
            free(node);
            break;
        }
        prev = node;
    }

    TlsUnlock();
}

SNSEND
