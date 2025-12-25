
#ifndef _SMatrix_DEFINED_
#define _SMatrix_DEFINED_

#include "SRect.h"
#include <interface/SRender-i.h>
#include <interface/SMatrix-i.h>

#define SToBool(cond) ((cond) != 0)

SNSBEGIN

/**
 * @class SMatrix
 * @brief The SMatrix class holds a 3x3 matrix for transforming coordinates.
 *        SMatrix does not have a constructor, so it must be explicitly initialized
 *        using either reset() - to construct an identity matrix, or one of the set
 *        functions (e.g., setTranslate, setRotate, etc.).
 */
class SOUI_EXP SMatrix
    : public IxForm
    , public IMatrix {
  public:
    /**
     * @brief Default constructor, initializes the matrix to identity.
     */
    SMatrix()
    {
        reset();
    }

    /**
     * @brief Constructor that initializes the matrix with the given data array.
     * @param data Array of 9 floats representing the matrix elements.
     */
    SMatrix(const float data[9], int type = kUnknown_Mask);

  public:
    /**
     * @brief Multiplies the matrix by another matrix.
     * @param src The matrix to multiply with.
     * @return Reference to the modified matrix.
     */
    SMatrix &operator*=(const SMatrix &src);

    /**
     * @brief Multiplies the matrix by another matrix.
     * @param src The matrix to multiply with.
     * @return New matrix resulting from the multiplication.
     */
    SMatrix operator*(const SMatrix &src) const;

    /**
     * @brief Assignment operator.
     * @param src The matrix to assign from.
     * @return Reference to the modified matrix.
     */
    SMatrix &operator=(const SMatrix &src);

  public:
    /**
     * @brief Post-concats the matrix with a translation.
     * @param dx Translation in the x-direction.
     * @param dy Translation in the y-direction.
     * @return Reference to the modified matrix.
     */
    SMatrix &translate(float dx, float dy)
    {
        postTranslate(dx, dy);
        return *this;
    }

    /**
     * @brief Post-concats the matrix with a scale.
     * @param sx Scale factor in the x-direction.
     * @param sy Scale factor in the y-direction.
     * @return Reference to the modified matrix.
     */
    SMatrix &scale(float sx, float sy)
    {
        postScale(sx, sy);
        return *this;
    }

    /**
     * @brief Post-concats the matrix with a shear.
     * @param sh Shear factor in the x-direction.
     * @param sv Shear factor in the y-direction.
     * @return Reference to the modified matrix.
     */
    SMatrix &shear(float sh, float sv)
    {
        postSkew(sh, sv);
        return *this;
    }

    /**
     * @brief Post-concats the matrix with a rotation.
     * @param deg Rotation angle in degrees.
     * @return Reference to the modified matrix.
     */
    SMatrix &rotate(float deg)
    {
        postRotate(deg);
        return *this;
    }

  public:
    /**
     * @brief Returns a pointer to the matrix data.
     * @return Pointer to the matrix data.
     */
    STDMETHOD_(IxForm *, Data)(THIS) SCONST OVERRIDE;

    /**
     * @brief Resets the matrix to the identity matrix.
     */
    STDMETHOD_(void, reset)(THIS) OVERRIDE;

    /**
     * @brief Sets the matrix to the identity matrix.
     */
    STDMETHOD_(void, setIdentity)(THIS) OVERRIDE;

    /**
     * @brief Checks if the matrix is the identity matrix.
     * @return True if the matrix is identity, false otherwise.
     */
    STDMETHOD_(BOOL, isIdentity)(THIS) SCONST OVERRIDE;

    /**
     * @brief Sets the matrix to a translation.
     * @param dx Translation in the x-direction.
     * @param dy Translation in the y-direction.
     */
    STDMETHOD_(void, setTranslate)(THIS_ float dx, float dy) OVERRIDE;

    /**
     * @brief Sets the matrix to a scale.
     * @param sx Scale factor in the x-direction.
     * @param sy Scale factor in the y-direction.
     */
    STDMETHOD_(void, setScale)(THIS_ float sx, float sy) OVERRIDE;

    /**
     * @brief Sets the matrix to a scale with pivot point.
     * @param sx Scale factor in the x-direction.
     * @param sy Scale factor in the y-direction.
     * @param px Pivot point x-coordinate.
     * @param py Pivot point y-coordinate.
     */
    STDMETHOD_(void, setScale2)(THIS_ float sx, float sy, float px, float py) OVERRIDE;

    /**
     * @brief Sets the matrix to a rotation.
     * @param degrees Rotation angle in degrees.
     */
    STDMETHOD_(void, setRotate)(THIS_ float degrees) OVERRIDE;

    /**
     * @brief Sets the matrix to a rotation with pivot point.
     * @param degrees Rotation angle in degrees.
     * @param px Pivot point x-coordinate.
     * @param py Pivot point y-coordinate.
     */
    STDMETHOD_(void, setRotate2)(THIS_ float degrees, float px, float py) OVERRIDE;

    /**
     * @brief Sets the matrix to a skew.
     * @param kx Skew factor in the x-direction.
     * @param ky Skew factor in the y-direction.
     */
    STDMETHOD_(void, setSkew)(THIS_ float kx, float ky) OVERRIDE;

    /**
     * @brief Sets the matrix to a skew with pivot point.
     * @param kx Skew factor in the x-direction.
     * @param ky Skew factor in the y-direction.
     * @param px Pivot point x-coordinate.
     * @param py Pivot point y-coordinate.
     */
    STDMETHOD_(void, setSkew2)(THIS_ float kx, float ky, float px, float py) OVERRIDE;

  public:
    /**
     * @enum TypeMask
     * @brief Enum of bit fields for the mask returned by getType().
     *        Use this to identify the complexity of the matrix.
     */
    enum TypeMask
    {
        kIdentity_Mask = 0,      //!< Set if the matrix is identity
        kTranslate_Mask = 0x01,  //!< Set if the matrix has translation
        kScale_Mask = 0x02,      //!< Set if the matrix has X or Y scale
        kAffine_Mask = 0x04,     //!< Set if the matrix skews or rotates
        kPerspective_Mask = 0x08 //!< Set if the matrix is in perspective
    };

    /**
     * @brief Returns a bitfield describing the transformations the matrix may perform.
     *        The bitfield is computed conservatively, so it may include false positives.
     *        For example, when kPerspective_Mask is true, all other bits may be set to true
     *        even in the case of a non-perspective transform.
     * @return Bitfield describing the transformations.
     */
    TypeMask getType() const
    {
        if (fTypeMask & kUnknown_Mask)
        {
            fTypeMask = this->computeTypeMask();
        }
        // only return the public masks
        return (TypeMask)(fTypeMask & 0xF);
    }

    /**
     * @brief Checks if the matrix contains only scale and translation.
     * @return True if the matrix contains only scale and translation, false otherwise.
     */
    bool isScaleTranslate() const
    {
        return !(this->getType() & ~(kScale_Mask | kTranslate_Mask));
    }

    /**
     * @brief Returns true if the matrix will map a rectangle to another rectangle.
     *        This can be true if the matrix is identity, scale-only, or rotates a multiple of 90 degrees.
     * @return True if the matrix preserves rectangle shape, false otherwise.
     */
    bool rectStaysRect() const
    {
        if (fTypeMask & kUnknown_Mask)
        {
            fTypeMask = this->computeTypeMask();
        }
        return (fTypeMask & kRectStaysRect_Mask) != 0;
    }

    /**
     * @brief Alias for rectStaysRect().
     * @return True if the matrix preserves rectangle shape, false otherwise.
     */
    bool preservesAxisAlignment() const
    {
        return this->rectStaysRect();
    }

    /**
     * @brief Checks if the matrix contains perspective elements.
     * @return True if the matrix has perspective elements, false otherwise.
     */
    bool hasPerspective() const
    {
        return SToBool(this->getPerspectiveTypeMaskOnly() & kPerspective_Mask);
    }

    /**
     * @brief Checks if the matrix contains only translation, rotation/reflection, or uniform scale.
     *        Returns false if other transformation types are included or if the matrix is degenerate.
     * @param tol Tolerance for floating-point comparison (default is SK_ScalarNearlyZero).
     * @return True if the matrix is a similarity transform, false otherwise.
     */
    bool isSimilarity(float tol = SK_ScalarNearlyZero) const;

    bool operator==(const SMatrix &other) const
    {
        return SFloatsEqual(fMat, other.fMat,9);
    }

    bool operator!=(const SMatrix &other) const
    {
        return !(*this == other);
    }

    /**
     * @brief Checks if the matrix contains only translation, rotation/reflection, or scale (non-uniform scale is allowed).
     *        Returns false if other transformation types are included or if the matrix is degenerate.
     * @param tol Tolerance for floating-point comparison (default is SK_ScalarNearlyZero).
     * @return True if the matrix preserves right angles, false otherwise.
     */
    bool preservesRightAngles(float tol = SK_ScalarNearlyZero) const;

    /**
     * @brief Enum for affine array indices.
     *        Affine arrays are in column major order because that's how PDF and XPS like it.
     */
    enum
    {
        kAScaleX,
        kASkewY,
        kASkewX,
        kAScaleY,
        kATransX,
        kATransY
    };

    /**
     * @brief Accesses the matrix element at the specified index.
     * @param index Index of the matrix element.
     * @return Matrix element value.
     */
    float operator[](int index) const
    {
        SASSERT((unsigned)index < 9);
        return fMat[index];
    }

    /**
     * @brief Accesses the matrix element at the specified index.
     * @param index Index of the matrix element.
     * @return Matrix element value.
     */
    float get(int index) const
    {
        SASSERT((unsigned)index < 9);
        return fMat[index];
    }

    /**
     * @brief Accesses the matrix element at the specified index.
     * @param index Index of the matrix element.
     * @return Reference to the matrix element.
     */
    float &operator[](int index)
    {
        SASSERT((unsigned)index < 9);
        this->setTypeMask(kUnknown_Mask);
        return fMat[index];
    }

    /**
     * @brief Sets the matrix element at the specified index.
     * @param index Index of the matrix element.
     * @param value New value for the matrix element.
     */
    void set(int index, float value)
    {
        SASSERT((unsigned)index < 9);
        fMat[index] = value;
        this->setTypeMask(kUnknown_Mask);
    }

    /**
     * @brief Sets the matrix to the specified data array.
     * @param data Array of 9 floats representing the matrix elements.
     * @param matType Type mask for the matrix (default is kUnknown_Mask).
     */
    void setMatrix(const float data[9], int matType = kUnknown_Mask);

    /**
     * @brief Sets the matrix to a translation.
     * @param v Translation vector.
     */
    void setTranslate(const SVector2D &v)
    {
        this->setTranslate(v.fX, v.fY);
    }

    /**
     * @brief Sets the matrix to scale by 1/divx and 1/divy.
     * @param divx Division factor in the x-direction.
     * @param divy Division factor in the y-direction.
     * @return True if successful, false if either divx or divy is zero.
     */
    bool setIDiv(int divx, int divy);

    /**
     * @brief Sets the matrix to rotate by the specified sine and cosine values, with a pivot point at (px, py).
     * @param sinValue Sine of the rotation angle.
     * @param cosValue Cosine of the rotation angle.
     * @param px Pivot point x-coordinate.
     * @param py Pivot point y-coordinate.
     */
    void setSinCos(float sinValue, float cosValue, float px, float py);

    /**
     * @brief Sets the matrix to rotate by the specified sine and cosine values.
     * @param sinValue Sine of the rotation angle.
     * @param cosValue Cosine of the rotation angle.
     */
    void setSinCos(float sinValue, float cosValue);

    /**
     * @brief Sets the matrix to the concatenation of the two specified matrices.
     * @param a First matrix.
     * @param b Second matrix.
     */
    void setConcat(const SMatrix &a, const SMatrix &b);

    /**
     * @brief Pre-concats the matrix with the specified translation.
     * @param dx Translation in the x-direction.
     * @param dy Translation in the y-direction.
     */
    void preTranslate(float dx, float dy);

    /**
     * @brief Pre-concats the matrix with the specified translation.
     * @param dx Translation in the x-direction.
     * @param dy Translation in the y-direction.
     */
    void preTranslate(int dx, int dy);

    /**
     * @brief Pre-concats the matrix with the specified scale.
     * @param sx Scale factor in the x-direction.
     * @param sy Scale factor in the y-direction.
     * @param px Pivot point x-coordinate.
     * @param py Pivot point y-coordinate.
     */
    void preScale(float sx, float sy, float px, float py);

    /**
     * @brief Pre-concats the matrix with the specified scale.
     * @param sx Scale factor in the x-direction.
     * @param sy Scale factor in the y-direction.
     */
    void preScale(float sx, float sy);

    /**
     * @brief Pre-concats the matrix with the specified rotation.
     * @param degrees Rotation angle in degrees.
     * @param px Pivot point x-coordinate.
     * @param py Pivot point y-coordinate.
     */
    void preRotate(float degrees, float px, float py);

    /**
     * @brief Pre-concats the matrix with the specified rotation.
     * @param degrees Rotation angle in degrees.
     */
    void preRotate(float degrees);

    /**
     * @brief Pre-concats the matrix with the specified skew.
     * @param kx Skew factor in the x-direction.
     * @param ky Skew factor in the y-direction.
     * @param px Pivot point x-coordinate.
     * @param py Pivot point y-coordinate.
     */
    void preSkew(float kx, float ky, float px, float py);

    /**
     * @brief Pre-concats the matrix with the specified skew.
     * @param kx Skew factor in the x-direction.
     * @param ky Skew factor in the y-direction.
     */
    void preSkew(float kx, float ky);

    /**
     * @brief Pre-concats the matrix with the specified matrix.
     * @param other Matrix to pre-concatenate.
     */
    void preConcat(const SMatrix &other);

    /**
     * @brief Post-concats the matrix with the specified translation.
     * @param dx Translation in the x-direction.
     * @param dy Translation in the y-direction.
     */
    void postTranslate(float dx, float dy);

    /**
     * @brief Post-concats the matrix with the specified translation.
     * @param dx Translation in the x-direction.
     * @param dy Translation in the y-direction.
     */
    void postTranslate(int dx, int dy);

    /**
     * @brief Post-concats the matrix with the specified scale.
     * @param sx Scale factor in the x-direction.
     * @param sy Scale factor in the y-direction.
     * @param px Pivot point x-coordinate.
     * @param py Pivot point y-coordinate.
     */
    void postScale(float sx, float sy, float px, float py);

    /**
     * @brief Post-concats the matrix with the specified scale.
     * @param sx Scale factor in the x-direction.
     * @param sy Scale factor in the y-direction.
     */
    void postScale(float sx, float sy);

    /**
     * @brief Post-concats the matrix by dividing it by the specified integers.
     * @param divx Division factor in the x-direction.
     * @param divy Division factor in the y-direction.
     * @return True if successful, false if either divx or divy is zero.
     */
    bool postIDiv(int divx, int divy);

    /**
     * @brief Post-concats the matrix with the specified rotation.
     * @param degrees Rotation angle in degrees.
     * @param px Pivot point x-coordinate.
     * @param py Pivot point y-coordinate.
     */
    void postRotate(float degrees, float px, float py);

    /**
     * @brief Post-concats the matrix with the specified rotation.
     * @param degrees Rotation angle in degrees.
     */
    void postRotate(float degrees);

    /**
     * @brief Post-concats the matrix with the specified skew.
     * @param kx Skew factor in the x-direction.
     * @param ky Skew factor in the y-direction.
     * @param px Pivot point x-coordinate.
     * @param py Pivot point y-coordinate.
     */
    void postSkew(float kx, float ky, float px, float py);

    /**
     * @brief Post-concats the matrix with the specified skew.
     * @param kx Skew factor in the x-direction.
     * @param ky Skew factor in the y-direction.
     */
    void postSkew(float kx, float ky);

    /**
     * @brief Post-concats the matrix with the specified matrix.
     * @param other Matrix to post-concatenate.
     */
    void postConcat(const SMatrix &other);

    /**
     * @enum ScaleToFit
     * @brief Enum for scale-to-fit options.
     */
    enum ScaleToFit
    {
        /**
         * Scale in X and Y independently, so that src matches dst exactly.
         * This may change the aspect ratio of the src.
         */
        kFill_ScaleToFit,
        /**
         * Compute a scale that will maintain the original src aspect ratio,
         * but will also ensure that src fits entirely inside dst. At least one
         * axis (X or Y) will fit exactly. kStart aligns the result to the
         * left and top edges of dst.
         */
        kStart_ScaleToFit,
        /**
         * Compute a scale that will maintain the original src aspect ratio,
         * but will also ensure that src fits entirely inside dst. At least one
         * axis (X or Y) will fit exactly. The result is centered inside dst.
         */
        kCenter_ScaleToFit,
        /**
         * Compute a scale that will maintain the original src aspect ratio,
         * but will also ensure that src fits entirely inside dst. At least one
         * axis (X or Y) will fit exactly. kEnd aligns the result to the
         * right and bottom edges of dst.
         */
        kEnd_ScaleToFit
    };

    /**
     * @brief Set the matrix to the scale and translate values that map the source
     *        rectangle to the destination rectangle, returning true if the result
     *        can be represented.
     * @param src Source rectangle to map from.
     * @param dst Destination rectangle to map to.
     * @param stf ScaleToFit option.
     * @return True if the matrix can be represented by the rectangle mapping.
     */
    bool setRectToRect(const SRect &src, const SRect &dst, ScaleToFit stf);

    /**
     * @brief Set the matrix such that the specified src points would map to the
     *        specified dst points. count must be within [0..4].
     * @param src Array of source points.
     * @param dst Array of destination points.
     * @param count Number of points to use for the transformation.
     * @return True if the matrix was set to the specified transformation.
     */
    bool setPolyToPoly(const SPoint src[], const SPoint dst[], int count);

    /**
     * @brief If this matrix can be inverted, return true and if inverse is not null,
     *        set inverse to be the inverse of this matrix. If this matrix cannot be
     *        inverted, ignore inverse and return false.
     * @param inverse Pointer to the matrix to store the inverse.
     * @return True if the matrix can be inverted, false otherwise.
     */
    bool invert(SMatrix *inverse) const
    {
        // Allow the trivial case to be inlined.
        if (this->isIdentity())
        {
            if (inverse)
            {
                inverse->reset();
            }
            return true;
        }
        return this->invertNonIdentity(inverse);
    }

    /**
     * @brief Fills the passed array with affine identity values in column major order.
     * @param affine Array to fill with affine identity values. Must not be NULL.
     */
    static void SetAffineIdentity(float affine[6]);

    /**
     * @brief Fills the passed array with the affine values in column major order.
     *        If the matrix is a perspective transform, returns false and does not
     *        change the passed array.
     * @param affine Array to fill with affine values. Ignored if NULL.
     * @return True if the matrix is affine, false otherwise.
     */
    bool asAffine(float affine[6]) const;

    /**
     * @brief Apply this matrix to the array of points specified by src, and write
     *        the transformed points into the array of points specified by dst.
     *        dst[] = M * src[]
     * @param dst Where the transformed coordinates are written. It must contain at least count entries.
     * @param src The original coordinates that are to be transformed. It must contain at least count entries.
     * @param count The number of points in src to read, and then transform into dst.
     */
    void mapPoints(SPoint dst[], const SPoint src[], int count) const;

    /**
     * @brief Apply this matrix to the array of points, overwriting it with the
     *        transformed values.
     *        dst[] = M * pts[]
     * @param pts The points to be transformed. It must contain at least count entries.
     * @param count The number of points in pts.
     */
    void mapPoints(SPoint pts[], int count) const
    {
        this->mapPoints(pts, pts, count);
    }

    /**
     * @brief Like mapPoints but with custom byte stride between the points. Stride
     *        should be a multiple of sizeof(float).
     * @param pts Array of points to transform.
     * @param stride Byte stride between points.
     * @param count Number of points to transform.
     */
    void mapPointsWithStride(SPoint pts[], size_t stride, int count) const
    {
        SASSERT(stride >= sizeof(SPoint));
        SASSERT(0 == stride % sizeof(float));
        for (int i = 0; i < count; ++i)
        {
            this->mapPoints(pts, pts, 1);
            pts = (SPoint *)((intptr_t)pts + stride);
        }
    }

    /**
     * @brief Like mapPoints but with custom byte stride between the points.
     * @param dst Array of transformed points.
     * @param src Array of original points.
     * @param stride Byte stride between points.
     * @param count Number of points to transform.
     */
    void mapPointsWithStride(SPoint dst[], SPoint src[], size_t stride, int count) const
    {
        SASSERT(stride >= sizeof(SPoint));
        SASSERT(0 == stride % sizeof(float));
        for (int i = 0; i < count; ++i)
        {
            this->mapPoints(dst, src, 1);
            src = (SPoint *)((intptr_t)src + stride);
            dst = (SPoint *)((intptr_t)dst + stride);
        }
    }

    /**
     * @brief Apply this matrix to the array of homogeneous points, specified by src,
     *        where a homogeneous point is defined by 3 contiguous scalar values,
     *        and write the transformed points into the array of scalars specified by dst.
     *        dst[] = M * src[]
     * @param dst Where the transformed coordinates are written. It must contain at least 3 * count entries.
     * @param src The original coordinates that are to be transformed. It must contain at least 3 * count entries.
     * @param count The number of triples (homogeneous points) in src to read, and then transform into dst.
     */
    void mapHomogeneousPoints(float dst[], const float src[], int count) const;

    /**
     * @brief Apply this matrix to the point (x, y) and store the result in *result.
     * @param x X-coordinate of the point.
     * @param y Y-coordinate of the point.
     * @param result Pointer to store the transformed point.
     */
    void mapXY(float x, float y, SPoint *result) const
    {
        SASSERT(result);
        this->getMapXYProc()(*this, x, y, result);
    }

    /**
     * @brief Apply this matrix to the array of vectors specified by src, and write
     *        the transformed vectors into the array of vectors specified by dst.
     *        This is similar to mapPoints, but ignores any translation in the matrix.
     * @param dst Where the transformed coordinates are written. It must contain at least count entries.
     * @param src The original coordinates that are to be transformed. It must contain at least count entries.
     * @param count The number of vectors in src to read, and then transform into dst.
     */
    void mapVectors(SVector2D dst[], const SVector2D src[], int count) const;

    /**
     * @brief Apply this matrix to the array of vectors specified by src, and write
     *        the transformed vectors into the array of vectors specified by dst.
     *        This is similar to mapPoints, but ignores any translation in the matrix.
     * @param vecs The vectors to be transformed. It must contain at least count entries.
     * @param count The number of vectors in vecs.
     */
    void mapVectors(SVector2D vecs[], int count) const
    {
        this->mapVectors(vecs, vecs, count);
    }

    /**
     * @brief Apply this matrix to the src rectangle, and write the transformed
     *        rectangle into dst. This is accomplished by transforming the 4 corners
     *        of src, and then setting dst to the bounds of those points.
     * @param dst Where the transformed rectangle is written.
     * @param src The original rectangle to be transformed.
     * @return The result of calling rectStaysRect().
     */
    bool mapRect(SRect *dst, const SRect &src) const;

    /**
     * @brief Apply this matrix to the rectangle, and write the transformed rectangle
     *        back into it. This is accomplished by transforming the 4 corners of
     *        rect, and then setting it to the bounds of those points.
     * @param rect The rectangle to transform.
     * @return The result of calling rectStaysRect().
     */
    bool mapRect(SRect *rect) const
    {
        return this->mapRect(rect, *rect);
    }

    /**
     * @brief Apply this matrix to the src rectangle, and write the four transformed
     *        points into dst. The points written to dst will be the original top-left, top-right,
     *        bottom-right, and bottom-left points transformed by the matrix.
     * @param dst Where the transformed quad is written.
     * @param rect The original rectangle to be transformed.
     */
    void mapRectToQuad(SPoint dst[4], const SRect &rect) const
    {
        // This could potentially be faster if we only transformed each x and y of the rect once.
        rect.toQuad(dst);
        this->mapPoints(dst, 4);
    }

    /**
     * @brief Return the mean radius of a circle after it has been mapped by
     *        this matrix. NOTE: in perspective this value assumes the circle
     *        has its center at the origin.
     * @param radius Original radius of the circle.
     * @return Mapped radius of the circle.
     */
    float mapRadius(float radius) const;

    /**
     * @typedef MapXYProc
     * @brief Function pointer type for mapping a single point (x, y) to a transformed point.
     */
    typedef void (*MapXYProc)(const SMatrix &mat, float x, float y, SPoint *result);

    /**
     * @brief Get the appropriate MapXYProc for the given type mask.
     * @param mask Type mask of the matrix.
     * @return Pointer to the MapXYProc function.
     */
    static MapXYProc GetMapXYProc(TypeMask mask)
    {
        SASSERT((mask & ~kAllMasks) == 0);
        return gMapXYProcs[mask & kAllMasks];
    }

    /**
     * @brief Get the appropriate MapXYProc for this matrix.
     * @return Pointer to the MapXYProc function.
     */
    MapXYProc getMapXYProc() const
    {
        return GetMapXYProc(this->getType());
    }

    /**
     * @typedef MapPtsProc
     * @brief Function pointer type for mapping an array of points.
     */
    typedef void (*MapPtsProc)(const SMatrix &mat, SPoint dst[], const SPoint src[], int count);

    /**
     * @brief Get the appropriate MapPtsProc for the given type mask.
     * @param mask Type mask of the matrix.
     * @return Pointer to the MapPtsProc function.
     */
    static MapPtsProc GetMapPtsProc(TypeMask mask)
    {
        SASSERT((mask & ~kAllMasks) == 0);
        return gMapPtsProcs[mask & kAllMasks];
    }

    /**
     * @brief Get the appropriate MapPtsProc for this matrix.
     * @return Pointer to the MapPtsProc function.
     */
    MapPtsProc getMapPtsProc() const
    {
        return GetMapPtsProc(this->getType());
    }

    /**
     * @brief Efficient comparison of two matrices. It distinguishes between zero and
     *        negative zero. It will return false when the sign of zero values is the
     *        only difference between the two matrices. It considers NaN values to be
     *        equal to themselves. So a matrix full of NaNs is "cheap equal" to
     *        another matrix full of NaNs iff the NaN values are bitwise identical
     *        while according to strict the strict == test a matrix with a NaN value
     *        is equal to nothing, including itself.
     * @param m Matrix to compare with.
     * @return True if matrices are cheaply equal, false otherwise.
     */
    bool cheapEqualTo(const SMatrix &m) const
    {
        return 0 == memcmp(fMat, m.fMat, sizeof(fMat));
    }

    enum
    {
        // writeTo/readFromMemory will never return a value larger than this
        kMaxFlattenSize = 9 * sizeof(float) + sizeof(uint32_t)
    };

    /**
     * @brief Calculates the minimum scaling factor of the matrix as computed from the SVD of the upper
     *        left 2x2. If the matrix has perspective -1 is returned.
     * @return Minimum scale factor.
     */
    float getMinScale() const;

    /**
     * @brief Calculates the maximum scaling factor of the matrix as computed from the SVD of the upper
     *        left 2x2. If the matrix has perspective -1 is returned.
     * @return Maximum scale factor.
     */
    float getMaxScale() const;

    /**
     * @brief Gets both the min and max scale factors. The min scale factor is scaleFactors[0] and the max
     *        is scaleFactors[1]. If the matrix has perspective false will be returned and scaleFactors
     *        will be unchanged.
     * @param scaleFactors Array to store the min and max scale factors.
     * @return True if the matrix does not have perspective, false otherwise.
     */
    bool getMinMaxScales(float scaleFactors[2]) const;

    /**
     * @brief Return a reference to a const identity matrix.
     * @return Reference to the identity matrix.
     */
    static const SMatrix &I();

    /**
     * @brief Return a reference to a const matrix that is "invalid", one that could
     *        never be used.
     * @return Reference to the invalid matrix.
     */
    static const SMatrix &InvalidMatrix();

    /**
     * @brief Return the concatenation of two matrices, a * b.
     * @param a First matrix.
     * @param b Second matrix.
     * @return Resulting matrix.
     */
    static SMatrix Concat(const SMatrix &a, const SMatrix &b)
    {
        SMatrix result;
        result.setConcat(a, b);
        return result;
    }

    /**
     * @brief Testing routine; the matrix's type cache should never need to be
     *        manually invalidated during normal use.
     */
    void dirtyMatrixTypeCache()
    {
        this->setTypeMask(kUnknown_Mask);
    }

  private:
    enum
    {
        /** Set if the matrix will map a rectangle to another rectangle. This
            can be true if the matrix is scale-only, or rotates a multiple of
            90 degrees.

            This bit will be set on identity matrices
        */
        kRectStaysRect_Mask = 0x10,

        /** Set if the perspective bit is valid even though the rest of
            the matrix is Unknown.
        */
        kOnlyPerspectiveValid_Mask = 0x40,

        kUnknown_Mask = 0x80,

        kORableMasks = kTranslate_Mask | kScale_Mask | kAffine_Mask | kPerspective_Mask,

        kAllMasks = kTranslate_Mask | kScale_Mask | kAffine_Mask | kPerspective_Mask | kRectStaysRect_Mask
    };

    mutable uint32_t fTypeMask;

    uint8_t computeTypeMask() const;
    uint8_t computePerspectiveTypeMask() const;

    void setTypeMask(int mask);

    void orTypeMask(int mask);

    void clearTypeMask(int mask)
    {
        // only allow a valid mask
        SASSERT((mask & kAllMasks) == mask);
        fTypeMask = fTypeMask & ~mask;
    }

    TypeMask getPerspectiveTypeMaskOnly() const
    {
        if ((fTypeMask & kUnknown_Mask) && !(fTypeMask & kOnlyPerspectiveValid_Mask))
        {
            fTypeMask = this->computePerspectiveTypeMask();
        }
        return (TypeMask)(fTypeMask & 0xF);
    }

    /** Returns true if we already know that the matrix is identity;
        false otherwise.
    */
    bool isTriviallyIdentity() const
    {
        if (fTypeMask & kUnknown_Mask)
        {
            return false;
        }
        return ((fTypeMask & 0xF) == 0);
    }

    bool invertNonIdentity(SMatrix *inverse) const;

    static bool Poly2Proc(const SPoint[], SMatrix *, const SPoint &scale);
    static bool Poly3Proc(const SPoint[], SMatrix *, const SPoint &scale);
    static bool Poly4Proc(const SPoint[], SMatrix *, const SPoint &scale);

    static void Identity_xy(const SMatrix &, float, float, SPoint *);
    static void Trans_xy(const SMatrix &, float, float, SPoint *);
    static void Scale_xy(const SMatrix &, float, float, SPoint *);
    static void ScaleTrans_xy(const SMatrix &, float, float, SPoint *);
    static void Rot_xy(const SMatrix &, float, float, SPoint *);
    static void RotTrans_xy(const SMatrix &, float, float, SPoint *);
    static void Persp_xy(const SMatrix &, float, float, SPoint *);

    static const MapXYProc gMapXYProcs[];

    static void Identity_pts(const SMatrix &, SPoint[], const SPoint[], int);
    static void Trans_pts(const SMatrix &, SPoint dst[], const SPoint[], int);
    static void Scale_pts(const SMatrix &, SPoint dst[], const SPoint[], int);
    static void ScaleTrans_pts(const SMatrix &, SPoint dst[], const SPoint[], int count);
    static void Rot_pts(const SMatrix &, SPoint dst[], const SPoint[], int);
    static void RotTrans_pts(const SMatrix &, SPoint dst[], const SPoint[], int count);
    static void Persp_pts(const SMatrix &, SPoint dst[], const SPoint[], int);
    static const MapPtsProc gMapPtsProcs[];

    friend class SkPerspIter;

  public:
    static float SFloatSinCos(float radians, float *cosValue);
};

SNSEND
#endif
