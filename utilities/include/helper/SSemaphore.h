#ifndef __SSEMAPHORE__H__
#define __SSEMAPHORE__H__

#include <utilities-def.h>
#include <helper/SNoCopyable.hpp>

SNSBEGIN

/**
 * @enum SemaphoreReturnCodes
 * @brief Return codes for semaphore operations.
 */
enum
{
    RETURN_OK = 0,       /**< Operation was successful. */
    RETURN_TIMEOUT = 1,  /**< Operation timed out. */
    RETURN_ERROR = -1    /**< An error occurred. */
};

class SemaphorePrivate;

/**
 * @class SSemaphore
 * @brief A class for thread synchronization using semaphores.
 * @details This class provides methods to wait for and notify semaphores.
 * @note This class is non-copyable.
 */
class UTILITIES_API SSemaphore : public SNoCopyable
{
public:
    /**
     * @brief Waits indefinitely for the semaphore to be notified.
     * @return RETURN_OK if the semaphore is notified successfully.
     * @return RETURN_ERROR if an error occurs.
     */
    int wait();

    /**
     * @brief Waits for the semaphore to be notified within a specified time.
     * @param msec The maximum time to wait in milliseconds.
     * @return RETURN_OK if the semaphore is notified successfully.
     * @return RETURN_TIMEOUT if the wait times out.
     * @return RETURN_ERROR if an error occurs.
     */
    int wait(unsigned int msec);

    /**
     * @brief Notifies the semaphore.
     * @details This function increases the semaphore count, allowing one or more waiting threads to proceed.
     */
    void notify();

    /**
     * @brief Default constructor.
     * Initializes the semaphore.
     */
    SSemaphore();

    /**
     * @brief Destructor.
     * Cleans up the semaphore resources.
     */
    virtual ~SSemaphore();

private:
    SemaphorePrivate &_private; /**< Pointer to the private implementation of the semaphore. */
};

SNSEND

#endif // __SSEMAPHORE__H__