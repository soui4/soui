#ifndef __STRANSFORMATION__H__
#define __STRANSFORMATION__H__

#include <interface/SRender-i.h>
#include <matrix/SMatrix.h>
#include <interface/STransform-i.h>

SNSBEGIN

/**
 * @class STransformation
 * @brief Defines the transformation to be applied at one point in time of an Animation.
 */
class SOUI_EXP STransformation : public ITransformation {
  public:
    /**
     * @brief Creates a new transformation with alpha = 1 and the identity matrix.
     */
    STransformation();

  protected:
    SMatrix mMatrix;         ///< The 3x3 matrix representing the transformation.
    BYTE mAlpha;             ///< The degree of transparency (255 means fully opaque, 0 means fully transparent).
    int mTransformationType; ///< Indicates the nature of this transformation.

  public:
    /**
     * @brief Gets the matrix representing the transformation.
     * @return Pointer to the matrix.
     */
    STDMETHOD_(IMatrix *, GetMatrix)(THIS) OVERRIDE;

    /**
     * @brief Gets the degree of transparency.
     * @return The alpha value (255 means fully opaque, 0 means fully transparent).
     */
    STDMETHOD_(BYTE, GetAlpha)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the degree of transparency.
     * @param alpha The alpha value (255 means fully opaque, 0 means fully transparent).
     */
    STDMETHOD_(void, SetAlpha)(THIS_ BYTE alpha) OVERRIDE;

    /**
     * @brief Composes this transformation with another transformation.
     * @param t Pointer to the transformation to compose with.
     */
    STDMETHOD_(void, Compose)(const ITransformation *t) OVERRIDE;

    /**
     * @brief Clears the transformation to the identity matrix and alpha to 1.
     */
    STDMETHOD_(void, Clear)(THIS) OVERRIDE;

    /**
     * @brief Sets the type of transformation.
     * @param type The type of transformation.
     */
    STDMETHOD_(void, SetTransformationType)(THIS_ int type) OVERRIDE;

  public:
    /**
     * @brief Indicates the nature of this transformation.
     * @return {@link #TYPE_ALPHA}, {@link #TYPE_MATRIX}, {@link #TYPE_BOTH}, or {@link #TYPE_IDENTITY}.
     */
    int getTransformationType() const;

    /**
     * @brief Clones the specified transformation.
     * @param t The transformation to clone.
     */
    void set(STransformation t);

    /**
     * @brief Applies this STransformation to an existing STransformation.
     * @param t The transformation to compose with.
     */
    void compose(const STransformation &t);

    /**
     * @brief Composes this transformation with another transformation using postConcat.
     * @param t The transformation to compose with.
     * @hide
     */
    void postCompose(STransformation t);

    /**
     * @brief Gets the 3x3 matrix representing the transformation.
     * @return Constant reference to the matrix.
     */
    const SMatrix &getMatrix() const;

    /**
     * @brief Gets the 3x3 matrix representing the transformation.
     * @return Reference to the matrix.
     */
    SMatrix &getMatrix();

    /**
     * @brief Sets the matrix representing the transformation.
     * @param mtx The matrix to set.
     */
    void setMatrix(const SMatrix &mtx);

    /**
     * @brief Updates the transformation type based on the current matrix and alpha.
     */
    void updateMatrixType();

    /**
     * @brief Checks if the transformation affects the alpha property.
     * @return TRUE if the transformation affects the alpha property, FALSE otherwise.
     */
    bool hasAlpha() const;

    /**
     * @brief Checks if the transformation affects the matrix property.
     * @return TRUE if the transformation affects the matrix property, FALSE otherwise.
     */
    bool hasMatrix() const;

    /**
     * @brief Checks if the transformation is the identity transformation.
     * @return TRUE if the transformation is the identity transformation, FALSE otherwise.
     */
    bool isIdentity() const;
};

SNSEND

#endif // __STRANSFORMATION__H__