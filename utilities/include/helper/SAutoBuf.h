/**
 * @file SAutoBuf.h
 * @brief Header file for the SAutoBuf class, a smart buffer management class.
 */

 #ifndef __SAUTOBUF__H__
 #define __SAUTOBUF__H__
 
 #include <utilities-def.h>
 
 SNSBEGIN
 
 /**
  * @class SAutoBuf
  * @brief A smart buffer management class that automatically handles memory allocation and deallocation.
  */
 class UTILITIES_API SAutoBuf
 {
 public:
	 /**
	  * @brief Default constructor.
	  * Initializes an empty buffer.
	  */
	 SAutoBuf();
 
	 /**
	  * @brief Constructor with initial buffer size.
	  * Allocates a buffer of the specified size.
	  * @param nElements The number of elements (bytes) to allocate.
	  */
	 SAutoBuf(size_t nElements);
 
	 /**
	  * @brief Destructor.
	  * Frees the allocated buffer if it was not externally attached.
	  */
	 ~SAutoBuf();
 
	 /**
	  * @brief Conversion operator to char*.
	  * @return Pointer to the managed buffer.
	  */
	 operator char *() const;
 
	 /**
	  * @brief Arrow operator to access members of the managed buffer.
	  * @return Pointer to the managed buffer.
	  */
	 char* operator ->() const;
 
	 /**
	  * @brief Const array subscript operator.
	  * @param i Index of the element to access.
	  * @return Const reference to the element at the specified index.
	  */
	 const char & operator[] (int i) const;
 
	 /**
	  * @brief Array subscript operator.
	  * @param i Index of the element to access.
	  * @return Reference to the element at the specified index.
	  */
	 char & operator[] (int i);
 
	 /**
	  * @brief Attaches an external buffer to the SAutoBuf object.
	  * The buffer will not be freed by the SAutoBuf object.
	  * @param pBuf Pointer to the external buffer.
	  * @param size Size of the external buffer.
	  */
	 void Attach(char *pBuf, size_t size);
 
	 /**
	  * @brief Detaches the managed buffer from the SAutoBuf object.
	  * The caller is responsible for freeing the buffer using soui_mem_wrapper::SouiFree.
	  * @return Pointer to the detached buffer.
	  */
	 char *Detach();
 
	 /**
	  * @brief Allocates a buffer of the specified size.
	  * If a buffer is already allocated, it will be freed first.
	  * @param nElements The number of elements (bytes) to allocate.
	  * @return Pointer to the newly allocated buffer.
	  */
	 char* Allocate(size_t nElements);
 
	 /**
	  * @brief Returns the size of the managed buffer.
	  * @return The size of the managed buffer.
	  */
	 size_t size();
 
	 /**
	  * @brief Frees the managed buffer.
	  * The buffer will be set to nullptr after being freed.
	  */
	 void Free();
 
 private:
	 char * m_pBuf;       /**< Pointer to the managed buffer. */
	 size_t m_nSize;      /**< Size of the managed buffer. */
	 bool   m_bExternalBuf; /**< Flag indicating if the buffer is externally attached. */
 };
 
 SNSEND
 
 #endif // __SAUTOBUF__H__