#ifndef __SBSTR_H__
#define __SBSTR_H__

#include <windows.h>
#include <oleauto.h>

typedef WCHAR char16;
typedef unsigned int uint32;

SNSBEGIN

// Manages a BSTR string pointer.
// The class interface is based on scoped_ptr.
class SOUI_EXP sbstr {
  public:
    /**
     * @brief Default constructor
     * Initializes the BSTR pointer to NULL.
     */
    sbstr()
        : bstr_(NULL)
    {
    }

    /**
     * @brief Constructor to create a new BSTR from a wide character string
     * @param non_bstr Pointer to the wide character string
     * NOTE: Do not pass a BSTR to this constructor expecting ownership to be transferred - even though it compiles! ;-)
     */
    explicit sbstr(const char16 *non_bstr);

    /**
     * @brief Constructor to create a new BSTR with a specified size from a wide character string
     * @param nSize Size of the BSTR in characters
     * @param non_bstr Pointer to the wide character string
     */
    explicit sbstr(int nSize, const char16 *non_bstr);

    /**
     * @brief Destructor
     * Releases the BSTR if it is not NULL.
     */
    ~sbstr();

    /**
     * @brief Give sbstr ownership over an already allocated BSTR or NULL
     * @param bstr Pointer to the BSTR to take ownership of
     * If you need to allocate a new BSTR instance, use |Allocate| instead.
     */
    void Reset(BSTR bstr = NULL);

    /**
     * @brief Releases ownership of the BSTR to the caller
     * @return Pointer to the released BSTR
     */
    BSTR Release();

    /**
     * @brief Creates a new BSTR from a wide character string
     * @param str Pointer to the wide character string
     * @return Pointer to the new BSTR, or NULL if allocation failed
     * If you already have a BSTR and want to transfer ownership to the sbstr instance, call |Reset| instead.
     */
    BSTR Allocate(const char16 *str);

    /**
     * @brief Allocates a new BSTR with the specified number of bytes
     * @param bytes Number of bytes to allocate
     * @return Pointer to the new BSTR, or NULL if allocation failed
     */
    BSTR AllocateBytes(size_t bytes);

    /**
     * @brief Sets the allocated length field of the already-allocated BSTR to be |bytes|
     * @param bytes Number of bytes to set as the length
     * This is useful when the BSTR was preallocated with e.g. SysAllocStringLen or SysAllocStringByteLen (call |AllocateBytes|) and then not all the bytes are being used.
     * NOTE: The actual allocated size of the BSTR MUST be >= bytes. That responsibility is with the caller.
     */
    void SetByteLen(size_t bytes);

    /**
     * @brief Swap values of two sbstr's
     * @param bstr2 Reference to the sbstr to swap with
     */
    void Swap(sbstr &bstr2);

    /**
     * @brief Retrieves the pointer address
     * @return Pointer to the BSTR pointer
     * Used to receive BSTRs as out arguments (and take ownership).
     * The function DCHECKs on the current value being NULL.
     * Usage: GetBstr(bstr.Receive());
     */
    BSTR *Receive();

    /**
     * @brief Returns the number of characters in the BSTR
     * @return Number of characters in the BSTR
     */
    size_t Length() const;

    /**
     * @brief Returns the number of bytes allocated for the BSTR
     * @return Number of bytes allocated for the BSTR
     */
    size_t ByteLength() const;

    /**
     * @brief Implicit conversion operator to BSTR
     * @return Pointer to the BSTR
     */
    operator BSTR() const
    {
        return bstr_;
    }

  protected:
    BSTR bstr_;

  private:
    // Forbid comparison of sbstr types. You should never have the same BSTR owned by two different sbstr instances.
    bool operator==(const sbstr &bstr2) const;
    bool operator!=(const sbstr &bstr2) const;
    DISALLOW_COPY_AND_ASSIGN(sbstr);
};

SNSEND

#endif //__SBSTR_H__