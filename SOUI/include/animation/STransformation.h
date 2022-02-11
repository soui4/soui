/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <interface/SRender-i.h>
#include <matrix/SMatrix.h>
#include <interface/STransform-i.h>

SNSBEGIN

/**
 * Defines the transformation to be applied at
 * one point in time of an Animation.
 *
 */
class SOUI_EXP STransformation : public ITransformation {
  public:
  protected:
    SMatrix mMatrix;
    BYTE mAlpha;
    int mTransformationType;

    /**
     * Creates a new transformation with alpha = 1 and the identity matrix.
     */
  public:
    STransformation();

  public:
    STDMETHOD_(IMatrix *, GetMatrix)(THIS) OVERRIDE;

    STDMETHOD_(BYTE, GetAlpha)(THIS) SCONST OVERRIDE;

    STDMETHOD_(void, SetAlpha)(THIS_ BYTE alpha) OVERRIDE;

    STDMETHOD_(void, Compose)(const ITransformation *t) OVERRIDE;

    STDMETHOD_(void, Clear)(THIS) OVERRIDE;

    STDMETHOD_(void, SetTransformationType)(THIS_ int type) OVERRIDE;

  public:
    /**
     * Indicates the nature of this transformation.
     *
     * @return {@link #TYPE_ALPHA}, {@link #TYPE_MATRIX},
     *         {@link #TYPE_BOTH} or {@link #TYPE_IDENTITY}.
     */
    int getTransformationType() const;

    /**
     * Clones the specified transformation.
     *
     * @param t The transformation to clone.
     */
    void set(STransformation t);

    /**
     * Apply this STransformation to an existing STransformation, e.g. apply
     * a scale effect to something that has already been rotated.
     * @param t
     */
    void compose(const STransformation &t);

    /**
     * Like {@link #compose(STransformation)} but does this.postConcat(t) of
     * the transformation matrix.
     * @hide
     */
    void postCompose(STransformation t);

    /**
     * @return The 3x3 Matrix representing the transformation to apply to the
     * coordinates of the object being animated
     */
    const SMatrix &getMatrix() const;

    SMatrix &getMatrix();

    void setMatrix(const SMatrix &mtx);
    /**
     * Sets the degree of transparency
     * @param alpha 255 means fully opaqe and 0 means fully transparent
     */

    void updateMatrixType();

    bool hasAlpha() const;

    bool hasMatrix() const;

    bool isIdentity() const;
};

SNSEND