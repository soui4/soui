/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkCanvasPriv.h"
#include "SkBitmapDevice.h"
#include "SkDeviceImageFilterProxy.h"
#include "SkDraw.h"
#include "SkDrawFilter.h"
#include "SkDrawLooper.h"
#include "SkImage.h"
#include "SkMetaData.h"
#include "SkPathOps.h"
#include "SkPatchUtils.h"
#include "SkPicture.h"
#include "SkRasterClip.h"
#include "SkRRect.h"
#include "SkSmallAllocator.h"
#include "SkSurface_Base.h"
#include "SkTemplates.h"
#include "SkTextBlob.h"
#include "SkTextFormatParams.h"
#include "SkTLazy.h"
#include "SkUtils.h"
#include "SkTypeface.h"

#if SK_SUPPORT_GPU
#include "GrRenderTarget.h"
#endif

// experimental for faster tiled drawing...
//#define SK_ENABLE_CLIP_QUICKREJECT

//#define SK_TRACE_SAVERESTORE

#ifdef SK_TRACE_SAVERESTORE
    static int gLayerCounter;
    static void inc_layer() { ++gLayerCounter; printf("----- inc layer %d\n", gLayerCounter); }
    static void dec_layer() { --gLayerCounter; printf("----- dec layer %d\n", gLayerCounter); }

    static int gRecCounter;
    static void inc_rec() { ++gRecCounter; printf("----- inc rec %d\n", gRecCounter); }
    static void dec_rec() { --gRecCounter; printf("----- dec rec %d\n", gRecCounter); }

    static int gCanvasCounter;
    static void inc_canvas() { ++gCanvasCounter; printf("----- inc canvas %d\n", gCanvasCounter); }
    static void dec_canvas() { --gCanvasCounter; printf("----- dec canvas %d\n", gCanvasCounter); }
#else
    #define inc_layer()
    #define dec_layer()
    #define inc_rec()
    #define dec_rec()
    #define inc_canvas()
    #define dec_canvas()
#endif

typedef SkTLazy<SkPaint> SkLazyPaint;

void SkCanvas::predrawNotify() {
    if (fSurfaceBase) {
        fSurfaceBase->aboutToDraw(SkSurface::kRetain_ContentChangeMode);
    }
}

///////////////////////////////////////////////////////////////////////////////

static uint32_t filter_paint_flags(const SkSurfaceProps& props, uint32_t flags) {
    const uint32_t propFlags = props.flags();
    if (propFlags & SkSurfaceProps::kDisallowDither_Flag) {
        flags &= ~SkPaint::kDither_Flag;
    }
    if (propFlags & SkSurfaceProps::kDisallowAntiAlias_Flag) {
        flags &= ~SkPaint::kAntiAlias_Flag;
    }
    return flags;
}

///////////////////////////////////////////////////////////////////////////////

/*  This is the record we keep for each SkBaseDevice that the user installs.
    The clip/matrix/proc are fields that reflect the top of the save/restore
    stack. Whenever the canvas changes, it marks a dirty flag, and then before
    these are used (assuming we're not on a layer) we rebuild these cache
    values: they reflect the top of the save stack, but translated and clipped
    by the device's XY offset and bitmap-bounds.
*/
struct DeviceCM {
    DeviceCM*           fNext;
    SkBaseDevice*       fDevice;
    SkRasterClip        fClip;
    const SkMatrix*     fMatrix;
    SkPaint*            fPaint; // may be null (in the future)

    DeviceCM(SkBaseDevice* device, int x, int y, const SkPaint* paint, SkCanvas* canvas,
             bool conservativeRasterClip)
        : fNext(NULL)
        , fClip(conservativeRasterClip)
    {
        if (NULL != device) {
            device->ref();
            device->onAttachToCanvas(canvas);
        }
        fDevice = device;
        fPaint = paint ? SkNEW_ARGS(SkPaint, (*paint)) : NULL;
    }

    ~DeviceCM() {
        if (fDevice) {
            fDevice->onDetachFromCanvas();
            fDevice->unref();
        }
        SkDELETE(fPaint);
    }

    void updateMC(const SkMatrix& totalMatrix, const SkRasterClip& totalClip,
                  const SkClipStack& clipStack, SkRasterClip* updateClip) {
        int x = fDevice->getOrigin().x();
        int y = fDevice->getOrigin().y();
        int width = fDevice->width();
        int height = fDevice->height();

        if ((x | y) == 0) {
            fMatrix = &totalMatrix;
            fClip = totalClip;
        } else {
            fMatrixStorage = totalMatrix;
            fMatrixStorage.postTranslate(SkIntToScalar(-x),
                                         SkIntToScalar(-y));
            fMatrix = &fMatrixStorage;

            totalClip.translate(-x, -y, &fClip);
        }

        fClip.op(SkIRect::MakeWH(width, height), SkRegion::kIntersect_Op);

        // intersect clip, but don't translate it (yet)

        if (updateClip) {
            updateClip->op(SkIRect::MakeXYWH(x, y, width, height),
                           SkRegion::kDifference_Op);
        }

        fDevice->setMatrixClip(*fMatrix, fClip.forceGetBW(), clipStack);

#ifdef SK_DEBUG
        if (!fClip.isEmpty()) {
            SkIRect deviceR;
            deviceR.set(0, 0, width, height);
            SkASSERT(deviceR.contains(fClip.getBounds()));
        }
#endif
    }

private:
    SkMatrix    fMatrixStorage;
};

/*  This is the record we keep for each save/restore level in the stack.
    Since a level optionally copies the matrix and/or stack, we have pointers
    for these fields. If the value is copied for this level, the copy is
    stored in the ...Storage field, and the pointer points to that. If the
    value is not copied for this level, we ignore ...Storage, and just point
    at the corresponding value in the previous level in the stack.
*/
class SkCanvas::MCRec {
public:
    SkRasterClip    fRasterClip;
    SkMatrix        fMatrix;
    SkDrawFilter*   fFilter;    // the current filter (or null)
    DeviceCM*       fLayer;
    /*  If there are any layers in the stack, this points to the top-most
        one that is at or below this level in the stack (so we know what
        bitmap/device to draw into from this level. This value is NOT
        reference counted, since the real owner is either our fLayer field,
        or a previous one in a lower level.)
    */
    DeviceCM*   fTopLayer;

    MCRec(bool conservativeRasterClip) : fRasterClip(conservativeRasterClip) {
        fMatrix.reset();
        fFilter     = NULL;
        fLayer      = NULL;
        fTopLayer   = NULL;

        // don't bother initializing fNext
        inc_rec();
    }
    MCRec(const MCRec& prev) : fRasterClip(prev.fRasterClip) {
        fMatrix = prev.fMatrix;
        fFilter = SkSafeRef(prev.fFilter);
        fLayer = NULL;
        fTopLayer = prev.fTopLayer;

        // don't bother initializing fNext
        inc_rec();
    }
    ~MCRec() {
        SkSafeUnref(fFilter);
        SkDELETE(fLayer);
        dec_rec();
    }
};

class SkDrawIter : public SkDraw {
public:
    SkDrawIter(SkCanvas* canvas, bool skipEmptyClips = true) {
        canvas = canvas->canvasForDrawIter();
        fCanvas = canvas;
        canvas->updateDeviceCMCache();

        fClipStack = &canvas->fClipStack;
        fCurrLayer = canvas->fMCRec->fTopLayer;
        fSkipEmptyClips = skipEmptyClips;
    }

    bool next() {
        // skip over recs with empty clips
        if (fSkipEmptyClips) {
            while (fCurrLayer && fCurrLayer->fClip.isEmpty()) {
                fCurrLayer = fCurrLayer->fNext;
            }
        }

        const DeviceCM* rec = fCurrLayer;
        if (rec && rec->fDevice) {

            fMatrix = rec->fMatrix;
            fClip   = &((SkRasterClip*)&rec->fClip)->forceGetBW();
            fRC     = &rec->fClip;
            fDevice = rec->fDevice;
            fBitmap = &fDevice->accessBitmap(true);
            fPaint  = rec->fPaint;
            SkDEBUGCODE(this->validate();)

            fCurrLayer = rec->fNext;
            // fCurrLayer may be NULL now

            return true;
        }
        return false;
    }

    SkBaseDevice* getDevice() const { return fDevice; }
    int getX() const { return fDevice->getOrigin().x(); }
    int getY() const { return fDevice->getOrigin().y(); }
    const SkMatrix& getMatrix() const { return *fMatrix; }
    const SkRegion& getClip() const { return *fClip; }
    const SkPaint* getPaint() const { return fPaint; }

private:
    SkCanvas*       fCanvas;
    const DeviceCM* fCurrLayer;
    const SkPaint*  fPaint;     // May be null.
    SkBool8         fSkipEmptyClips;

    typedef SkDraw INHERITED;
};

/////////////////////////////////////////////////////////////////////////////

class AutoDrawLooper {
public:
    AutoDrawLooper(SkCanvas* canvas, const SkSurfaceProps& props, const SkPaint& paint,
                   bool skipLayerForImageFilter = false,
                   const SkRect* bounds = NULL) : fOrigPaint(paint) {
        fCanvas = canvas;
        fFilter = canvas->getDrawFilter();
        fPaint = &fOrigPaint;
        fSaveCount = canvas->getSaveCount();
        fDoClearImageFilter = false;
        fDone = false;

        if (!skipLayerForImageFilter && fOrigPaint.getImageFilter()) {
            SkPaint tmp;
            tmp.setImageFilter(fOrigPaint.getImageFilter());
            (void)canvas->internalSaveLayer(bounds, &tmp, SkCanvas::kARGB_ClipLayer_SaveFlag,
                                            true, SkCanvas::kFullLayer_SaveLayerStrategy);
            // we'll clear the imageFilter for the actual draws in next(), so
            // it will only be applied during the restore().
            fDoClearImageFilter = true;
        }

        if (SkDrawLooper* looper = paint.getLooper()) {
            void* buffer = fLooperContextAllocator.reserveT<SkDrawLooper::Context>(
                    looper->contextSize());
            fLooperContext = looper->createContext(canvas, buffer);
            fIsSimple = false;
        } else {
            fLooperContext = NULL;
            // can we be marked as simple?
            fIsSimple = !fFilter && !fDoClearImageFilter;
        }

        uint32_t oldFlags = paint.getFlags();
        fNewPaintFlags = filter_paint_flags(props, oldFlags);
        if (fIsSimple && (fNewPaintFlags != oldFlags)) {
            SkPaint* paint = fLazyPaint.set(fOrigPaint);
            paint->setFlags(fNewPaintFlags);
            fPaint = paint;
            // if we're not simple, doNext() will take care of calling setFlags()
        }
    }

    ~AutoDrawLooper() {
        if (fDoClearImageFilter) {
            fCanvas->internalRestore();
        }
        SkASSERT(fCanvas->getSaveCount() == fSaveCount);
    }

    const SkPaint& paint() const {
        SkASSERT(fPaint);
        return *fPaint;
    }

    bool next(SkDrawFilter::Type drawType) {
        if (fDone) {
            return false;
        } else if (fIsSimple) {
            fDone = true;
            return !fPaint->nothingToDraw();
        } else {
            return this->doNext(drawType);
        }
    }

private:
    SkLazyPaint     fLazyPaint;
    SkCanvas*       fCanvas;
    const SkPaint&  fOrigPaint;
    SkDrawFilter*   fFilter;
    const SkPaint*  fPaint;
    int             fSaveCount;
    uint32_t        fNewPaintFlags;
    bool            fDoClearImageFilter;
    bool            fDone;
    bool            fIsSimple;
    SkDrawLooper::Context* fLooperContext;
    SkSmallAllocator<1, 32> fLooperContextAllocator;

    bool doNext(SkDrawFilter::Type drawType);
};

bool AutoDrawLooper::doNext(SkDrawFilter::Type drawType) {
    fPaint = NULL;
    SkASSERT(!fIsSimple);
    SkASSERT(fLooperContext || fFilter || fDoClearImageFilter);

    SkPaint* paint = fLazyPaint.set(fOrigPaint);
    paint->setFlags(fNewPaintFlags);

    if (fDoClearImageFilter) {
        paint->setImageFilter(NULL);
    }

    if (fLooperContext && !fLooperContext->next(fCanvas, paint)) {
        fDone = true;
        return false;
    }
    if (fFilter) {
        if (!fFilter->filter(paint, drawType)) {
            fDone = true;
            return false;
        }
        if (NULL == fLooperContext) {
            // no looper means we only draw once
            fDone = true;
        }
    }
    fPaint = paint;

    // if we only came in here for the imagefilter, mark us as done
    if (!fLooperContext && !fFilter) {
        fDone = true;
    }

    // call this after any possible paint modifiers
    if (fPaint->nothingToDraw()) {
        fPaint = NULL;
        return false;
    }
    return true;
}

////////// macros to place around the internal draw calls //////////////////

#define LOOPER_BEGIN_DRAWDEVICE(paint, type)                        \
    this->predrawNotify();                                          \
    AutoDrawLooper  looper(this, fProps, paint, true);              \
    while (looper.next(type)) {                                     \
        SkDrawIter          iter(this);

#define LOOPER_BEGIN(paint, type, bounds)                           \
    this->predrawNotify();                                          \
    AutoDrawLooper  looper(this, fProps, paint, false, bounds);     \
    while (looper.next(type)) {                                     \
        SkDrawIter          iter(this);

#define LOOPER_END    }

////////////////////////////////////////////////////////////////////////////

void SkCanvas::setupDevice(SkBaseDevice* device) {
    device->setPixelGeometry(fProps.pixelGeometry());
}

SkBaseDevice* SkCanvas::init(SkBaseDevice* device, InitFlags flags) {
    fConservativeRasterClip = SkToBool(flags & kConservativeRasterClip_InitFlag);
    fCachedLocalClipBounds.setEmpty();
    fCachedLocalClipBoundsDirty = true;
    fAllowSoftClip = true;
    fAllowSimplifyClip = false;
    fDeviceCMDirty = true;
    fSaveLayerCount = 0;
    fCullCount = 0;
    fMetaData = NULL;

    fMCRec = (MCRec*)fMCStack.push_back();
    new (fMCRec) MCRec(fConservativeRasterClip);

    fMCRec->fLayer = SkNEW_ARGS(DeviceCM, (NULL, 0, 0, NULL, NULL, fConservativeRasterClip));
    fMCRec->fTopLayer = fMCRec->fLayer;

    fSurfaceBase = NULL;

    if (device) {
        this->setupDevice(device);
        if (device->forceConservativeRasterClip()) {
            fConservativeRasterClip = true;
        }
        device->onAttachToCanvas(this);
        fMCRec->fLayer->fDevice = SkRef(device);
        fMCRec->fRasterClip.setRect(SkIRect::MakeWH(device->width(), device->height()));
    }
    return device;
}

SkCanvas::SkCanvas()
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(SkSurfaceProps::kLegacyFontHost_InitType)
{
    inc_canvas();

    this->init(NULL, kDefault_InitFlags);
}

static SkBitmap make_nopixels(int width, int height) {
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::MakeUnknown(width, height));
    return bitmap;
}

class SkNoPixelsBitmapDevice : public SkBitmapDevice {
public:
    SkNoPixelsBitmapDevice(int width, int height) : INHERITED(make_nopixels(width, height)) {}

private:

    typedef SkBitmapDevice INHERITED;
};

SkCanvas::SkCanvas(int width, int height)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(SkSurfaceProps::kLegacyFontHost_InitType)
{
    inc_canvas();

    this->init(SkNEW_ARGS(SkNoPixelsBitmapDevice, (width, height)), kDefault_InitFlags)->unref();
}

SkCanvas::SkCanvas(int width, int height, InitFlags flags)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(SkSurfaceProps::kLegacyFontHost_InitType)
{
    inc_canvas();

    this->init(SkNEW_ARGS(SkNoPixelsBitmapDevice, (width, height)), flags)->unref();
}

SkCanvas::SkCanvas(SkBaseDevice* device, const SkSurfaceProps* props, InitFlags flags)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(SkSurfacePropsCopyOrDefault(props))
{
    inc_canvas();

    this->init(device, flags);
}

SkCanvas::SkCanvas(SkBaseDevice* device)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(SkSurfaceProps::kLegacyFontHost_InitType)
{
    inc_canvas();

    this->init(device, kDefault_InitFlags);
}

SkCanvas::SkCanvas(const SkBitmap& bitmap, const SkSurfaceProps& props)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(props)
{
    inc_canvas();

    SkAutoTUnref<SkBaseDevice> device(SkNEW_ARGS(SkBitmapDevice, (bitmap)));
    this->init(device, kDefault_InitFlags);
}

SkCanvas::SkCanvas(const SkBitmap& bitmap)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
    , fProps(SkSurfaceProps::kLegacyFontHost_InitType)
{
    inc_canvas();

    SkAutoTUnref<SkBaseDevice> device(SkNEW_ARGS(SkBitmapDevice, (bitmap)));
    this->init(device, kDefault_InitFlags);
}

SkCanvas::~SkCanvas() {
    // free up the contents of our deque
    this->restoreToCount(1);    // restore everything but the last
    SkASSERT(0 == fSaveLayerCount);

    this->internalRestore();    // restore the last, since we're going away

    SkDELETE(fMetaData);

    dec_canvas();
}

SkDrawFilter* SkCanvas::getDrawFilter() const {
    return fMCRec->fFilter;
}

SkDrawFilter* SkCanvas::setDrawFilter(SkDrawFilter* filter) {
    SkRefCnt_SafeAssign(fMCRec->fFilter, filter);
    return filter;
}

SkMetaData& SkCanvas::getMetaData() {
    // metadata users are rare, so we lazily allocate it. If that changes we
    // can decide to just make it a field in the device (rather than a ptr)
    if (NULL == fMetaData) {
        fMetaData = new SkMetaData;
    }
    return *fMetaData;
}

///////////////////////////////////////////////////////////////////////////////

void SkCanvas::flush() {
    SkBaseDevice* device = this->getDevice();
    if (device) {
        device->flush();
    }
}

SkISize SkCanvas::getTopLayerSize() const {
    SkBaseDevice* d = this->getTopDevice();
    return d ? SkISize::Make(d->width(), d->height()) : SkISize::Make(0, 0);
}

SkIPoint SkCanvas::getTopLayerOrigin() const {
    SkBaseDevice* d = this->getTopDevice();
    return d ? d->getOrigin() : SkIPoint::Make(0, 0);
}

SkISize SkCanvas::getBaseLayerSize() const {
    SkBaseDevice* d = this->getDevice();
    return d ? SkISize::Make(d->width(), d->height()) : SkISize::Make(0, 0);
}

SkBaseDevice* SkCanvas::getDevice() const {
    // return root device
    MCRec* rec = (MCRec*) fMCStack.front();
    SkASSERT(rec && rec->fLayer);
    return rec->fLayer->fDevice;
}

SkBaseDevice* SkCanvas::getTopDevice(bool updateMatrixClip) const {
    if (updateMatrixClip) {
        const_cast<SkCanvas*>(this)->updateDeviceCMCache();
    }
    return fMCRec->fTopLayer->fDevice;
}

SkBaseDevice* SkCanvas::setRootDevice(SkBaseDevice* device) {
    // return root device
    SkDeque::F2BIter iter(fMCStack);
    MCRec*           rec = (MCRec*)iter.next();
    SkASSERT(rec && rec->fLayer);
    SkBaseDevice*    rootDevice = rec->fLayer->fDevice;

    if (rootDevice == device) {
        return device;
    }

    if (device) {
        device->onAttachToCanvas(this);
    }
    if (rootDevice) {
        rootDevice->onDetachFromCanvas();
    }

    SkRefCnt_SafeAssign(rec->fLayer->fDevice, device);
    rootDevice = device;
    this->setupDevice(device);

    fDeviceCMDirty = true;

    /*  Now we update our initial region to have the bounds of the new device,
        and then intersect all of the clips in our stack with these bounds,
        to ensure that we can't draw outside of the device's bounds (and trash
                                                                     memory).

    NOTE: this is only a partial-fix, since if the new device is larger than
        the previous one, we don't know how to "enlarge" the clips in our stack,
        so drawing may be artificially restricted. Without keeping a history of
        all calls to canvas->clipRect() and canvas->clipPath(), we can't exactly
        reconstruct the correct clips, so this approximation will have to do.
        The caller really needs to restore() back to the base if they want to
        accurately take advantage of the new device bounds.
    */

    SkIRect bounds;
    if (device) {
        bounds.set(0, 0, device->width(), device->height());
    } else {
        bounds.setEmpty();
    }
    // now jam our 1st clip to be bounds, and intersect the rest with that
    rec->fRasterClip.setRect(bounds);
    while ((rec = (MCRec*)iter.next()) != NULL) {
        (void)rec->fRasterClip.op(bounds, SkRegion::kIntersect_Op);
    }

    return device;
}

bool SkCanvas::readPixels(SkBitmap* bitmap, int x, int y) {
    if (kUnknown_SkColorType == bitmap->colorType() || bitmap->getTexture()) {
        return false;
    }

    bool weAllocated = false;
    if (NULL == bitmap->pixelRef()) {
        if (!bitmap->tryAllocPixels()) {
            return false;
        }
        weAllocated = true;
    }

    SkBitmap bm(*bitmap);
    bm.lockPixels();
    if (bm.getPixels() && this->readPixels(bm.info(), bm.getPixels(), bm.rowBytes(), x, y)) {
        return true;
    }

    if (weAllocated) {
        bitmap->setPixelRef(NULL);
    }
    return false;
}

bool SkCanvas::readPixels(const SkIRect& srcRect, SkBitmap* bitmap) {
    SkIRect r = srcRect;
    const SkISize size = this->getBaseLayerSize();
    if (!r.intersect(0, 0, size.width(), size.height())) {
        bitmap->reset();
        return false;
    }

    if (!bitmap->tryAllocN32Pixels(r.width(), r.height())) {
        // bitmap will already be reset.
        return false;
    }
    if (!this->readPixels(bitmap->info(), bitmap->getPixels(), bitmap->rowBytes(), r.x(), r.y())) {
        bitmap->reset();
        return false;
    }
    return true;
}

bool SkCanvas::readPixels(const SkImageInfo& origInfo, void* dstP, size_t rowBytes, int x, int y) {
    switch (origInfo.colorType()) {
        case kUnknown_SkColorType:
        case kIndex_8_SkColorType:
            return false;
        default:
            break;
    }
    if (NULL == dstP || rowBytes < origInfo.minRowBytes()) {
        return false;
    }
    if (0 == origInfo.width() || 0 == origInfo.height()) {
        return false;
    }

    SkBaseDevice* device = this->getDevice();
    if (!device) {
        return false;
    }

    const SkISize size = this->getBaseLayerSize();
    SkIRect srcR = SkIRect::MakeXYWH(x, y, origInfo.width(), origInfo.height());
    if (!srcR.intersect(0, 0, size.width(), size.height())) {
        return false;
    }

    // the intersect may have shrunk info's logical size
    const SkImageInfo info = origInfo.makeWH(srcR.width(), srcR.height());

    // if x or y are negative, then we have to adjust pixels
    if (x > 0) {
        x = 0;
    }
    if (y > 0) {
        y = 0;
    }
    // here x,y are either 0 or negative
    dstP = ((char*)dstP - y * rowBytes - x * info.bytesPerPixel());

    // The device can assert that the requested area is always contained in its bounds
    return device->readPixels(info, dstP, rowBytes, srcR.x(), srcR.y());
}

bool SkCanvas::writePixels(const SkBitmap& bitmap, int x, int y) {
    if (bitmap.getTexture()) {
        return false;
    }
    SkBitmap bm(bitmap);
    bm.lockPixels();
    if (bm.getPixels()) {
        return this->writePixels(bm.info(), bm.getPixels(), bm.rowBytes(), x, y);
    }
    return false;
}

bool SkCanvas::writePixels(const SkImageInfo& origInfo, const void* pixels, size_t rowBytes,
                           int x, int y) {
    switch (origInfo.colorType()) {
        case kUnknown_SkColorType:
        case kIndex_8_SkColorType:
            return false;
        default:
            break;
    }
    if (NULL == pixels || rowBytes < origInfo.minRowBytes()) {
        return false;
    }

    const SkISize size = this->getBaseLayerSize();
    SkIRect target = SkIRect::MakeXYWH(x, y, origInfo.width(), origInfo.height());
    if (!target.intersect(0, 0, size.width(), size.height())) {
        return false;
    }

    SkBaseDevice* device = this->getDevice();
    if (!device) {
        return false;
    }

    // the intersect may have shrunk info's logical size
    const SkImageInfo info = origInfo.makeWH(target.width(), target.height());

    // if x or y are negative, then we have to adjust pixels
    if (x > 0) {
        x = 0;
    }
    if (y > 0) {
        y = 0;
    }
    // here x,y are either 0 or negative
    pixels = ((const char*)pixels - y * rowBytes - x * info.bytesPerPixel());

    // Tell our owning surface to bump its generation ID
    this->predrawNotify();

    // The device can assert that the requested area is always contained in its bounds
    return device->writePixels(info, pixels, rowBytes, target.x(), target.y());
}

SkCanvas* SkCanvas::canvasForDrawIter() {
    return this;
}

//////////////////////////////////////////////////////////////////////////////

void SkCanvas::updateDeviceCMCache() {
    if (fDeviceCMDirty) {
        const SkMatrix& totalMatrix = this->getTotalMatrix();
        const SkRasterClip& totalClip = fMCRec->fRasterClip;
        DeviceCM*       layer = fMCRec->fTopLayer;

        if (NULL == layer->fNext) {   // only one layer
            layer->updateMC(totalMatrix, totalClip, fClipStack, NULL);
        } else {
            SkRasterClip clip(totalClip);
            do {
                layer->updateMC(totalMatrix, clip, fClipStack, &clip);
            } while ((layer = layer->fNext) != NULL);
        }
        fDeviceCMDirty = false;
    }
}

///////////////////////////////////////////////////////////////////////////////

int SkCanvas::internalSave() {
    int saveCount = this->getSaveCount(); // record this before the actual save

    MCRec* newTop = (MCRec*)fMCStack.push_back();
    new (newTop) MCRec(*fMCRec);    // balanced in restore()
    fMCRec = newTop;

    fClipStack.save();

    return saveCount;
}

int SkCanvas::save() {
    this->willSave();
    return this->internalSave();
}

static bool bounds_affects_clip(SkCanvas::SaveFlags flags) {
#ifdef SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG
    return (flags & SkCanvas::kClipToLayer_SaveFlag) != 0;
#else
    return true;
#endif
}

bool SkCanvas::clipRectBounds(const SkRect* bounds, SaveFlags flags,
                               SkIRect* intersection, const SkImageFilter* imageFilter) {
    SkIRect clipBounds;
    SkRegion::Op op = SkRegion::kIntersect_Op;
    if (!this->getClipDeviceBounds(&clipBounds)) {
        return false;
    }

    if (imageFilter) {
        imageFilter->filterBounds(clipBounds, fMCRec->fMatrix, &clipBounds);
        // Filters may grow the bounds beyond the device bounds.
        op = SkRegion::kReplace_Op;
    }
    SkIRect ir;
    if (bounds) {
        SkRect r;

        this->getTotalMatrix().mapRect(&r, *bounds);
        r.roundOut(&ir);
        // early exit if the layer's bounds are clipped out
        if (!ir.intersect(clipBounds)) {
            if (bounds_affects_clip(flags)) {
                fMCRec->fRasterClip.setEmpty();
            }
            return false;
        }
    } else {    // no user bounds, so just use the clip
        ir = clipBounds;
    }

    if (bounds_affects_clip(flags)) {
        fClipStack.clipDevRect(ir, op);
        // early exit if the clip is now empty
        if (!fMCRec->fRasterClip.op(ir, op)) {
            return false;
        }
    }

    if (intersection) {
        *intersection = ir;
    }
    return true;
}

int SkCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint) {
    SaveLayerStrategy strategy = this->willSaveLayer(bounds, paint, kARGB_ClipLayer_SaveFlag);
    return this->internalSaveLayer(bounds, paint, kARGB_ClipLayer_SaveFlag, false, strategy);
}

int SkCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint,
                        SaveFlags flags) {
    SaveLayerStrategy strategy = this->willSaveLayer(bounds, paint, flags);
    return this->internalSaveLayer(bounds, paint, flags, false, strategy);
}

int SkCanvas::internalSaveLayer(const SkRect* bounds, const SkPaint* paint, SaveFlags flags,
                                bool justForImageFilter, SaveLayerStrategy strategy) {
#ifndef SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG
    flags |= kClipToLayer_SaveFlag;
#endif

    // do this before we create the layer. We don't call the public save() since
    // that would invoke a possibly overridden virtual
    int count = this->internalSave();

    fDeviceCMDirty = true;

    SkIRect ir;
    if (!this->clipRectBounds(bounds, flags, &ir, paint ? paint->getImageFilter() : NULL)) {
        return count;
    }

    // FIXME: do willSaveLayer() overriders returning kNoLayer_SaveLayerStrategy really care about
    // the clipRectBounds() call above?
    if (kNoLayer_SaveLayerStrategy == strategy) {
        return count;
    }

    // Kill the imagefilter if our device doesn't allow it
    SkLazyPaint lazyP;
    if (paint && paint->getImageFilter()) {
        if (!this->getTopDevice()->allowImageFilter(paint->getImageFilter())) {
            if (justForImageFilter) {
                // early exit if the layer was just for the imageFilter
                return count;
            }
            SkPaint* p = lazyP.set(*paint);
            p->setImageFilter(NULL);
            paint = p;
        }
    }

    bool isOpaque = !SkToBool(flags & kHasAlphaLayer_SaveFlag);
    SkImageInfo info = SkImageInfo::MakeN32(ir.width(), ir.height(),
                        isOpaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType);

    SkBaseDevice* device;
    if (paint && paint->getImageFilter()) {
        device = this->getDevice();
        if (device) {
            device = device->createCompatibleDeviceForImageFilter(info);
        }
    } else {
        device = this->createLayerDevice(info);
    }
    if (NULL == device) {
        SkDebugf("Unable to create device for layer.");
        return count;
    }
    this->setupDevice(device);

    device->setOrigin(ir.fLeft, ir.fTop);
    DeviceCM* layer = SkNEW_ARGS(DeviceCM,
                                 (device, ir.fLeft, ir.fTop, paint, this, fConservativeRasterClip));
    device->unref();

    layer->fNext = fMCRec->fTopLayer;
    fMCRec->fLayer = layer;
    fMCRec->fTopLayer = layer;    // this field is NOT an owner of layer

    fSaveLayerCount += 1;
    return count;
}

int SkCanvas::saveLayerAlpha(const SkRect* bounds, U8CPU alpha) {
    return this->saveLayerAlpha(bounds, alpha, kARGB_ClipLayer_SaveFlag);
}

int SkCanvas::saveLayerAlpha(const SkRect* bounds, U8CPU alpha,
                             SaveFlags flags) {
    if (0xFF == alpha) {
        return this->saveLayer(bounds, NULL, flags);
    } else {
        SkPaint tmpPaint;
        tmpPaint.setAlpha(alpha);
        return this->saveLayer(bounds, &tmpPaint, flags);
    }
}

void SkCanvas::restore() {
    // check for underflow
    if (fMCStack.count() > 1) {
        this->willRestore();
        this->internalRestore();
        this->didRestore();
    }
}

void SkCanvas::internalRestore() {
    SkASSERT(fMCStack.count() != 0);

    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;

    fClipStack.restore();

    // reserve our layer (if any)
    DeviceCM* layer = fMCRec->fLayer;   // may be null
    // now detach it from fMCRec so we can pop(). Gets freed after its drawn
    fMCRec->fLayer = NULL;

    // now do the normal restore()
    fMCRec->~MCRec();       // balanced in save()
    fMCStack.pop_back();
    fMCRec = (MCRec*)fMCStack.back();

    /*  Time to draw the layer's offscreen. We can't call the public drawSprite,
        since if we're being recorded, we don't want to record this (the
        recorder will have already recorded the restore).
    */
    if (layer) {
        if (layer->fNext) {
            const SkIPoint& origin = layer->fDevice->getOrigin();
            this->internalDrawDevice(layer->fDevice, origin.x(), origin.y(),
                                     layer->fPaint);
            // reset this, since internalDrawDevice will have set it to true
            fDeviceCMDirty = true;

            SkASSERT(fSaveLayerCount > 0);
            fSaveLayerCount -= 1;
        }
        SkDELETE(layer);
    }
}

int SkCanvas::getSaveCount() const {
    return fMCStack.count();
}

void SkCanvas::restoreToCount(int count) {
    // sanity check
    if (count < 1) {
        count = 1;
    }

    int n = this->getSaveCount() - count;
    for (int i = 0; i < n; ++i) {
        this->restore();
    }
}

bool SkCanvas::isDrawingToLayer() const {
    return fSaveLayerCount > 0;
}

SkSurface* SkCanvas::newSurface(const SkImageInfo& info, const SkSurfaceProps* props) {
    if (NULL == props) {
        props = &fProps;
    }
    return this->onNewSurface(info, *props);
}

SkSurface* SkCanvas::onNewSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    SkBaseDevice* dev = this->getDevice();
    return dev ? dev->newSurface(info, props) : NULL;
}

SkImageInfo SkCanvas::imageInfo() const {
    SkBaseDevice* dev = this->getDevice();
    if (dev) {
        return dev->imageInfo();
    } else {
        return SkImageInfo::MakeUnknown(0, 0);
    }
}

const void* SkCanvas::peekPixels(SkImageInfo* info, size_t* rowBytes) {
    return this->onPeekPixels(info, rowBytes);
}

const void* SkCanvas::onPeekPixels(SkImageInfo* info, size_t* rowBytes) {
    SkBaseDevice* dev = this->getDevice();
    return dev ? dev->peekPixels(info, rowBytes) : NULL;
}

void* SkCanvas::accessTopLayerPixels(SkImageInfo* info, size_t* rowBytes, SkIPoint* origin) {
    void* pixels = this->onAccessTopLayerPixels(info, rowBytes);
    if (pixels && origin) {
        *origin = this->getTopDevice(false)->getOrigin();
    }
    return pixels;
}

void* SkCanvas::onAccessTopLayerPixels(SkImageInfo* info, size_t* rowBytes) {
    SkBaseDevice* dev = this->getTopDevice();
    return dev ? dev->accessPixels(info, rowBytes) : NULL;
}

SkAutoROCanvasPixels::SkAutoROCanvasPixels(SkCanvas* canvas) {
    fAddr = canvas->peekPixels(&fInfo, &fRowBytes);
    if (NULL == fAddr) {
        fInfo = canvas->imageInfo();
        if (kUnknown_SkColorType == fInfo.colorType() || !fBitmap.tryAllocPixels(fInfo)) {
            return; // failure, fAddr is NULL
        }
        if (!canvas->readPixels(&fBitmap, 0, 0)) {
            return; // failure, fAddr is NULL
        }
        fAddr = fBitmap.getPixels();
        fRowBytes = fBitmap.rowBytes();
    }
    SkASSERT(fAddr);    // success
}

bool SkAutoROCanvasPixels::asROBitmap(SkBitmap* bitmap) const {
    if (fAddr) {
        return bitmap->installPixels(fInfo, const_cast<void*>(fAddr), fRowBytes);
    } else {
        bitmap->reset();
        return false;
    }
}

void SkCanvas::onPushCull(const SkRect& cullRect) {
    // do nothing. Subclasses may do something
}

void SkCanvas::onPopCull() {
    // do nothing. Subclasses may do something
}

/////////////////////////////////////////////////////////////////////////////
#ifdef SK_DEBUG
// Ensure that cull rects are monotonically nested in device space.
void SkCanvas::validateCull(const SkIRect& devCull) {
    if (fCullStack.isEmpty()
        || devCull.isEmpty()
        || fCullStack.top().contains(devCull)) {
        return;
    }

    SkDEBUGF(("Invalid cull: [%d %d %d %d] (previous cull: [%d %d %d %d])\n",
              devCull.x(), devCull.y(), devCull.right(), devCull.bottom(),
              fCullStack.top().x(), fCullStack.top().y(),
              fCullStack.top().right(), fCullStack.top().bottom()));

#ifdef ASSERT_NESTED_CULLING
    SkDEBUGFAIL("Invalid cull.");
#endif
}
#endif

void SkCanvas::pushCull(const SkRect& cullRect) {
    ++fCullCount;
    this->onPushCull(cullRect);

#ifdef SK_DEBUG
    // Map the cull rect into device space.
    SkRect mappedCull;
    this->getTotalMatrix().mapRect(&mappedCull, cullRect);

    // Take clipping into account.
    SkIRect devClip, devCull;
    mappedCull.roundOut(&devCull);
    this->getClipDeviceBounds(&devClip);
    if (!devCull.intersect(devClip)) {
        devCull.setEmpty();
    }

    this->validateCull(devCull);
    fCullStack.push(devCull); // balanced in popCull
#endif
}

void SkCanvas::popCull() {
    SkASSERT(fCullStack.count() == fCullCount);

    if (fCullCount > 0) {
        --fCullCount;
        this->onPopCull();

        SkDEBUGCODE(fCullStack.pop());
    }
}

/////////////////////////////////////////////////////////////////////////////

void SkCanvas::internalDrawBitmap(const SkBitmap& bitmap,
                                const SkMatrix& matrix, const SkPaint* paint) {
    if (bitmap.drawsNothing()) {
        return;
    }

    SkLazyPaint lazy;
    if (NULL == paint) {
        paint = lazy.init();
    }

    SkDEBUGCODE(bitmap.validate();)

    SkRect storage;
    const SkRect* bounds = NULL;
    if (paint && paint->canComputeFastBounds()) {
        bitmap.getBounds(&storage);
        matrix.mapRect(&storage);
        bounds = &paint->computeFastBounds(storage, &storage);
    }

    LOOPER_BEGIN(*paint, SkDrawFilter::kBitmap_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawBitmap(iter, bitmap, matrix, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::internalDrawDevice(SkBaseDevice* srcDev, int x, int y,
                                  const SkPaint* paint) {
    SkPaint tmp;
    if (NULL == paint) {
        tmp.setDither(true);
        paint = &tmp;
    }

    LOOPER_BEGIN_DRAWDEVICE(*paint, SkDrawFilter::kBitmap_Type)
    while (iter.next()) {
        SkBaseDevice* dstDev = iter.fDevice;
        paint = &looper.paint();
        SkImageFilter* filter = paint->getImageFilter();
        SkIPoint pos = { x - iter.getX(), y - iter.getY() };
        if (filter && !dstDev->canHandleImageFilter(filter)) {
            SkDeviceImageFilterProxy proxy(dstDev);
            SkBitmap dst;
            SkIPoint offset = SkIPoint::Make(0, 0);
            const SkBitmap& src = srcDev->accessBitmap(false);
            SkMatrix matrix = *iter.fMatrix;
            matrix.postTranslate(SkIntToScalar(-pos.x()), SkIntToScalar(-pos.y()));
            SkIRect clipBounds = SkIRect::MakeWH(srcDev->width(), srcDev->height());
            SkAutoTUnref<SkImageFilter::Cache> cache(dstDev->getImageFilterCache());
            SkImageFilter::Context ctx(matrix, clipBounds, cache.get());
            if (filter->filterImage(&proxy, src, ctx, &dst, &offset)) {
                SkPaint tmpUnfiltered(*paint);
                tmpUnfiltered.setImageFilter(NULL);
                dstDev->drawSprite(iter, dst, pos.x() + offset.x(), pos.y() + offset.y(),
                                   tmpUnfiltered);
            }
        } else {
            dstDev->drawDevice(iter, srcDev, pos.x(), pos.y(), *paint);
        }
    }
    LOOPER_END
}

void SkCanvas::drawSprite(const SkBitmap& bitmap, int x, int y,
                          const SkPaint* paint) {
    if (bitmap.drawsNothing()) {
        return;
    }
    SkDEBUGCODE(bitmap.validate();)

    SkPaint tmp;
    if (NULL == paint) {
        paint = &tmp;
    }

    LOOPER_BEGIN_DRAWDEVICE(*paint, SkDrawFilter::kBitmap_Type)

    while (iter.next()) {
        paint = &looper.paint();
        SkImageFilter* filter = paint->getImageFilter();
        SkIPoint pos = { x - iter.getX(), y - iter.getY() };
        if (filter && !iter.fDevice->canHandleImageFilter(filter)) {
            SkDeviceImageFilterProxy proxy(iter.fDevice);
            SkBitmap dst;
            SkIPoint offset = SkIPoint::Make(0, 0);
            SkMatrix matrix = *iter.fMatrix;
            matrix.postTranslate(SkIntToScalar(-pos.x()), SkIntToScalar(-pos.y()));
            SkIRect clipBounds = SkIRect::MakeWH(bitmap.width(), bitmap.height());
            SkAutoTUnref<SkImageFilter::Cache> cache(iter.fDevice->getImageFilterCache());
            SkImageFilter::Context ctx(matrix, clipBounds, cache.get());
            if (filter->filterImage(&proxy, bitmap, ctx, &dst, &offset)) {
                SkPaint tmpUnfiltered(*paint);
                tmpUnfiltered.setImageFilter(NULL);
                iter.fDevice->drawSprite(iter, dst, pos.x() + offset.x(), pos.y() + offset.y(),
                                         tmpUnfiltered);
            }
        } else {
            iter.fDevice->drawSprite(iter, bitmap, pos.x(), pos.y(), *paint);
        }
    }
    LOOPER_END
}

/////////////////////////////////////////////////////////////////////////////
void SkCanvas::translate(SkScalar dx, SkScalar dy) {
    SkMatrix m;
    m.setTranslate(dx, dy);
    this->concat(m);
}

void SkCanvas::scale(SkScalar sx, SkScalar sy) {
    SkMatrix m;
    m.setScale(sx, sy);
    this->concat(m);
}

void SkCanvas::rotate(SkScalar degrees) {
    SkMatrix m;
    m.setRotate(degrees);
    this->concat(m);
}

void SkCanvas::skew(SkScalar sx, SkScalar sy) {
    SkMatrix m;
    m.setSkew(sx, sy);
    this->concat(m);
}

void SkCanvas::concat(const SkMatrix& matrix) {
    if (matrix.isIdentity()) {
        return;
    }

    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;
    fMCRec->fMatrix.preConcat(matrix);

    this->didConcat(matrix);
}

void SkCanvas::setMatrix(const SkMatrix& matrix) {
    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;
    fMCRec->fMatrix = matrix;
    this->didSetMatrix(matrix);
}

void SkCanvas::resetMatrix() {
    SkMatrix matrix;

    matrix.reset();
    this->setMatrix(matrix);
}

//////////////////////////////////////////////////////////////////////////////

void SkCanvas::clipRect(const SkRect& rect, SkRegion::Op op, bool doAA) {
    ClipEdgeStyle edgeStyle = doAA ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;
    this->onClipRect(rect, op, edgeStyle);
}

void SkCanvas::onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
#ifdef SK_ENABLE_CLIP_QUICKREJECT
    if (SkRegion::kIntersect_Op == op) {
        if (fMCRec->fRasterClip.isEmpty()) {
            return false;
        }

        if (this->quickReject(rect)) {
            fDeviceCMDirty = true;
            fCachedLocalClipBoundsDirty = true;

            fClipStack.clipEmpty();
            return fMCRec->fRasterClip.setEmpty();
        }
    }
#endif

    AutoValidateClip avc(this);

    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;
    if (!fAllowSoftClip) {
        edgeStyle = kHard_ClipEdgeStyle;
    }

    if (fMCRec->fMatrix.rectStaysRect()) {
        // for these simpler matrices, we can stay a rect even after applying
        // the matrix. This means we don't have to a) make a path, and b) tell
        // the region code to scan-convert the path, only to discover that it
        // is really just a rect.
        SkRect      r;

        fMCRec->fMatrix.mapRect(&r, rect);
        fClipStack.clipDevRect(r, op, kSoft_ClipEdgeStyle == edgeStyle);
        fMCRec->fRasterClip.op(r, this->getBaseLayerSize(), op, kSoft_ClipEdgeStyle == edgeStyle);
    } else {
        // since we're rotated or some such thing, we convert the rect to a path
        // and clip against that, since it can handle any matrix. However, to
        // avoid recursion in the case where we are subclassed (e.g. Pictures)
        // we explicitly call "our" version of clipPath.
        SkPath  path;

        path.addRect(rect);
        this->SkCanvas::onClipPath(path, op, edgeStyle);
    }
}

static void rasterclip_path(SkRasterClip* rc, const SkCanvas* canvas, const SkPath& devPath,
                            SkRegion::Op op, bool doAA) {
    rc->op(devPath, canvas->getBaseLayerSize(), op, doAA);
}

void SkCanvas::clipRRect(const SkRRect& rrect, SkRegion::Op op, bool doAA) {
    ClipEdgeStyle edgeStyle = doAA ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;
    if (rrect.isRect()) {
        this->onClipRect(rrect.getBounds(), op, edgeStyle);
    } else {
        this->onClipRRect(rrect, op, edgeStyle);
    }
}

void SkCanvas::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    SkRRect transformedRRect;
    if (rrect.transform(fMCRec->fMatrix, &transformedRRect)) {
        AutoValidateClip avc(this);

        fDeviceCMDirty = true;
        fCachedLocalClipBoundsDirty = true;
        if (!fAllowSoftClip) {
            edgeStyle = kHard_ClipEdgeStyle;
        }

        fClipStack.clipDevRRect(transformedRRect, op, kSoft_ClipEdgeStyle == edgeStyle);

        SkPath devPath;
        devPath.addRRect(transformedRRect);

        rasterclip_path(&fMCRec->fRasterClip, this, devPath, op, kSoft_ClipEdgeStyle == edgeStyle);
        return;
    }

    SkPath path;
    path.addRRect(rrect);
    // call the non-virtual version
    this->SkCanvas::onClipPath(path, op, edgeStyle);
}

void SkCanvas::clipPath(const SkPath& path, SkRegion::Op op, bool doAA) {
    ClipEdgeStyle edgeStyle = doAA ? kSoft_ClipEdgeStyle : kHard_ClipEdgeStyle;
    SkRect r;
    if (!path.isInverseFillType() && path.isRect(&r)) {
        this->onClipRect(r, op, edgeStyle);
    } else {
        this->onClipPath(path, op, edgeStyle);
    }
}

void SkCanvas::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
#ifdef SK_ENABLE_CLIP_QUICKREJECT
    if (SkRegion::kIntersect_Op == op && !path.isInverseFillType()) {
        if (fMCRec->fRasterClip.isEmpty()) {
            return false;
        }

        if (this->quickReject(path.getBounds())) {
            fDeviceCMDirty = true;
            fCachedLocalClipBoundsDirty = true;

            fClipStack.clipEmpty();
            return fMCRec->fRasterClip.setEmpty();
        }
    }
#endif

    AutoValidateClip avc(this);

    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;
    if (!fAllowSoftClip) {
        edgeStyle = kHard_ClipEdgeStyle;
    }

    SkPath devPath;
    path.transform(fMCRec->fMatrix, &devPath);

    // Check if the transfomation, or the original path itself
    // made us empty. Note this can also happen if we contained NaN
    // values. computing the bounds detects this, and will set our
    // bounds to empty if that is the case. (see SkRect::set(pts, count))
    if (devPath.getBounds().isEmpty()) {
        // resetting the path will remove any NaN or other wanky values
        // that might upset our scan converter.
        devPath.reset();
    }

    // if we called path.swap() we could avoid a deep copy of this path
    fClipStack.clipDevPath(devPath, op, kSoft_ClipEdgeStyle == edgeStyle);

    if (fAllowSimplifyClip) {
        devPath.reset();
        devPath.setFillType(SkPath::kInverseEvenOdd_FillType);
        const SkClipStack* clipStack = getClipStack();
        SkClipStack::Iter iter(*clipStack, SkClipStack::Iter::kBottom_IterStart);
        const SkClipStack::Element* element;
        while ((element = iter.next())) {
            SkClipStack::Element::Type type = element->getType();
            SkPath operand;
            if (type != SkClipStack::Element::kEmpty_Type) {
                element->asPath(&operand);
            }
            SkRegion::Op elementOp = element->getOp();
            if (elementOp == SkRegion::kReplace_Op) {
                devPath = operand;
            } else {
                Op(devPath, operand, (SkPathOp) elementOp, &devPath);
            }
            // if the prev and curr clips disagree about aa -vs- not, favor the aa request.
            // perhaps we need an API change to avoid this sort of mixed-signals about
            // clipping.
            if (element->isAA()) {
                edgeStyle = kSoft_ClipEdgeStyle;
            }
        }
        op = SkRegion::kReplace_Op;
    }

    rasterclip_path(&fMCRec->fRasterClip, this, devPath, op, edgeStyle);
}

void SkCanvas::clipRegion(const SkRegion& rgn, SkRegion::Op op) {
    this->onClipRegion(rgn, op);
}

void SkCanvas::onClipRegion(const SkRegion& rgn, SkRegion::Op op) {
    AutoValidateClip avc(this);

    fDeviceCMDirty = true;
    fCachedLocalClipBoundsDirty = true;

    // todo: signal fClipStack that we have a region, and therefore (I guess)
    // we have to ignore it, and use the region directly?
    fClipStack.clipDevRect(rgn.getBounds(), op);

    fMCRec->fRasterClip.op(rgn, op);
}

#ifdef SK_DEBUG
void SkCanvas::validateClip() const {
    // construct clipRgn from the clipstack
    const SkBaseDevice* device = this->getDevice();
    if (!device) {
        SkASSERT(this->isClipEmpty());
        return;
    }

    SkIRect ir;
    ir.set(0, 0, device->width(), device->height());
    SkRasterClip tmpClip(ir, fConservativeRasterClip);

    SkClipStack::B2TIter                iter(fClipStack);
    const SkClipStack::Element* element;
    while ((element = iter.next()) != NULL) {
        switch (element->getType()) {
            case SkClipStack::Element::kRect_Type:
                element->getRect().round(&ir);
                tmpClip.op(ir, element->getOp());
                break;
            case SkClipStack::Element::kEmpty_Type:
                tmpClip.setEmpty();
                break;
            default: {
                SkPath path;
                element->asPath(&path);
                rasterclip_path(&tmpClip, this, path, element->getOp(), element->isAA());
                break;
            }
        }
    }
}
#endif

void SkCanvas::replayClips(ClipVisitor* visitor) const {
    SkClipStack::B2TIter                iter(fClipStack);
    const SkClipStack::Element*         element;

    while ((element = iter.next()) != NULL) {
        element->replay(visitor);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool SkCanvas::isClipEmpty() const {
    return fMCRec->fRasterClip.isEmpty();
}

bool SkCanvas::isClipRect() const {
    return fMCRec->fRasterClip.isRect();
}

bool SkCanvas::quickReject(const SkRect& rect) const {
    if (!rect.isFinite())
        return true;

    if (fMCRec->fRasterClip.isEmpty()) {
        return true;
    }

    if (fMCRec->fMatrix.hasPerspective()) {
        SkRect dst;
        fMCRec->fMatrix.mapRect(&dst, rect);
        SkIRect idst;
        dst.roundOut(&idst);
        return !SkIRect::Intersects(idst, fMCRec->fRasterClip.getBounds());
    } else {
        const SkRect& clipR = this->getLocalClipBounds();

        // for speed, do the most likely reject compares first
        // TODO: should we use | instead, or compare all 4 at once?
        if (rect.fTop >= clipR.fBottom || rect.fBottom <= clipR.fTop) {
            return true;
        }
        if (rect.fLeft >= clipR.fRight || rect.fRight <= clipR.fLeft) {
            return true;
        }
        return false;
    }
}

bool SkCanvas::quickReject(const SkPath& path) const {
    return path.isEmpty() || this->quickReject(path.getBounds());
}

bool SkCanvas::getClipBounds(SkRect* bounds) const {
    SkIRect ibounds;
    if (!this->getClipDeviceBounds(&ibounds)) {
        return false;
    }

    SkMatrix inverse;
    // if we can't invert the CTM, we can't return local clip bounds
    if (!fMCRec->fMatrix.invert(&inverse)) {
        if (bounds) {
            bounds->setEmpty();
        }
        return false;
    }

    if (bounds) {
        SkRect r;
        // adjust it outwards in case we are antialiasing
        const int inset = 1;

        r.iset(ibounds.fLeft - inset, ibounds.fTop - inset,
               ibounds.fRight + inset, ibounds.fBottom + inset);
        inverse.mapRect(bounds, r);
    }
    return true;
}

bool SkCanvas::getClipDeviceBounds(SkIRect* bounds) const {
    const SkRasterClip& clip = fMCRec->fRasterClip;
    if (clip.isEmpty()) {
        if (bounds) {
            bounds->setEmpty();
        }
        return false;
    }

    if (bounds) {
        *bounds = clip.getBounds();
    }
    return true;
}

const SkMatrix& SkCanvas::getTotalMatrix() const {
    return fMCRec->fMatrix;
}

const SkRegion& SkCanvas::internal_private_getTotalClip() const {
    return fMCRec->fRasterClip.forceGetBW();
}

GrRenderTarget* SkCanvas::internal_private_accessTopLayerRenderTarget() {
    SkBaseDevice* dev = this->getTopDevice();
    return dev ? dev->accessRenderTarget() : NULL;
}

SkBaseDevice* SkCanvas::createLayerDevice(const SkImageInfo& info) {
    SkBaseDevice* device = this->getTopDevice();
    return device ? device->createCompatibleDeviceForSaveLayer(info) : NULL;
}

GrContext* SkCanvas::getGrContext() {
#if SK_SUPPORT_GPU
    SkBaseDevice* device = this->getTopDevice();
    if (device) {
        GrRenderTarget* renderTarget = device->accessRenderTarget();
        if (renderTarget) {
            return renderTarget->getContext();
        }
    }
#endif

    return NULL;

}

void SkCanvas::drawDRRect(const SkRRect& outer, const SkRRect& inner,
                          const SkPaint& paint) {
    if (outer.isEmpty()) {
        return;
    }
    if (inner.isEmpty()) {
        this->drawRRect(outer, paint);
        return;
    }

    // We don't have this method (yet), but technically this is what we should
    // be able to assert...
    // SkASSERT(outer.contains(inner));
    //
    // For now at least check for containment of bounds
    SkASSERT(outer.getBounds().contains(inner.getBounds()));

    this->onDrawDRRect(outer, inner, paint);
}

//////////////////////////////////////////////////////////////////////////////
//  These are the virtual drawing methods
//////////////////////////////////////////////////////////////////////////////

void SkCanvas::clear(SkColor color) {
    SkDrawIter  iter(this);
    this->predrawNotify();
    while (iter.next()) {
        iter.fDevice->clear(color);
    }
}

void SkCanvas::onDiscard() {
    if (fSurfaceBase) {
        fSurfaceBase->aboutToDraw(SkSurface::kDiscard_ContentChangeMode);
    }
}

void SkCanvas::drawPaint(const SkPaint& paint) {
    this->internalDrawPaint(paint);
}

void SkCanvas::internalDrawPaint(const SkPaint& paint) {
    LOOPER_BEGIN(paint, SkDrawFilter::kPaint_Type, NULL)

    while (iter.next()) {
        iter.fDevice->drawPaint(iter, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                          const SkPaint& paint) {
    if ((long)count <= 0) {
        return;
    }

    SkRect r, storage;
    const SkRect* bounds = NULL;
    if (paint.canComputeFastBounds()) {
        // special-case 2 points (common for drawing a single line)
        if (2 == count) {
            r.set(pts[0], pts[1]);
        } else {
            r.set(pts, SkToInt(count));
        }
        bounds = &paint.computeFastStrokeBounds(r, &storage);
        if (this->quickReject(*bounds)) {
            return;
        }
    }

    SkASSERT(pts != NULL);

    LOOPER_BEGIN(paint, SkDrawFilter::kPoint_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawPoints(iter, mode, count, pts, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::drawRect(const SkRect& r, const SkPaint& paint) {
    SkRect storage;
    const SkRect* bounds = NULL;
    if (paint.canComputeFastBounds()) {
        bounds = &paint.computeFastBounds(r, &storage);
        if (this->quickReject(*bounds)) {
            return;
        }
    }

    LOOPER_BEGIN(paint, SkDrawFilter::kRect_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawRect(iter, r, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::drawOval(const SkRect& oval, const SkPaint& paint) {
    SkRect storage;
    const SkRect* bounds = NULL;
    if (paint.canComputeFastBounds()) {
        bounds = &paint.computeFastBounds(oval, &storage);
        if (this->quickReject(*bounds)) {
            return;
        }
    }

    LOOPER_BEGIN(paint, SkDrawFilter::kOval_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawOval(iter, oval, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    SkRect storage;
    const SkRect* bounds = NULL;
    if (paint.canComputeFastBounds()) {
        bounds = &paint.computeFastBounds(rrect.getBounds(), &storage);
        if (this->quickReject(*bounds)) {
            return;
        }
    }

    if (rrect.isRect()) {
        // call the non-virtual version
        this->SkCanvas::drawRect(rrect.getBounds(), paint);
        return;
    } else if (rrect.isOval()) {
        // call the non-virtual version
        this->SkCanvas::drawOval(rrect.getBounds(), paint);
        return;
    }

    LOOPER_BEGIN(paint, SkDrawFilter::kRRect_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawRRect(iter, rrect, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                            const SkPaint& paint) {
    SkRect storage;
    const SkRect* bounds = NULL;
    if (paint.canComputeFastBounds()) {
        bounds = &paint.computeFastBounds(outer.getBounds(), &storage);
        if (this->quickReject(*bounds)) {
            return;
        }
    }

    LOOPER_BEGIN(paint, SkDrawFilter::kRRect_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawDRRect(iter, outer, inner, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    if (!path.isFinite()) {
        return;
    }

    SkRect storage;
    const SkRect* bounds = NULL;
    if (!path.isInverseFillType() && paint.canComputeFastBounds()) {
        const SkRect& pathBounds = path.getBounds();
        bounds = &paint.computeFastBounds(pathBounds, &storage);
        if (this->quickReject(*bounds)) {
            return;
        }
    }

    const SkRect& r = path.getBounds();
    if (r.width() <= 0 && r.height() <= 0) {
        if (path.isInverseFillType()) {
            this->internalDrawPaint(paint);
        }
        return;
    }

    LOOPER_BEGIN(paint, SkDrawFilter::kPath_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawPath(iter, path, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::drawImage(const SkImage* image, SkScalar left, SkScalar top,
                       const SkPaint* paint) {
    image->draw(this, left, top, paint);
}

void SkCanvas::drawImageRect(const SkImage* image, const SkRect* src,
                           const SkRect& dst,
                           const SkPaint* paint) {
    image->draw(this, src, dst, paint);
}

void SkCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                          const SkPaint* paint) {
    SkDEBUGCODE(bitmap.validate();)

    if (NULL == paint || paint->canComputeFastBounds()) {
        SkRect bounds = {
            x, y,
            x + SkIntToScalar(bitmap.width()),
            y + SkIntToScalar(bitmap.height())
        };
        if (paint) {
            (void)paint->computeFastBounds(bounds, &bounds);
        }
        if (this->quickReject(bounds)) {
            return;
        }
    }

    SkMatrix matrix;
    matrix.setTranslate(x, y);
    this->internalDrawBitmap(bitmap, matrix, paint);
}

// this one is non-virtual, so it can be called safely by other canvas apis
void SkCanvas::internalDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src,
                                      const SkRect& dst, const SkPaint* paint,
                                      DrawBitmapRectFlags flags) {
    if (bitmap.drawsNothing() || dst.isEmpty()) {
        return;
    }

    SkRect storage;
    const SkRect* bounds = &dst;
    if (NULL == paint || paint->canComputeFastBounds()) {
        if (paint) {
            bounds = &paint->computeFastBounds(dst, &storage);
        }
        if (this->quickReject(*bounds)) {
            return;
        }
    }

    SkLazyPaint lazy;
    if (NULL == paint) {
        paint = lazy.init();
    }

    LOOPER_BEGIN(*paint, SkDrawFilter::kBitmap_Type, bounds)

    while (iter.next()) {
        iter.fDevice->drawBitmapRect(iter, bitmap, src, dst, looper.paint(), flags);
    }

    LOOPER_END
}

void SkCanvas::drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                    const SkRect& dst, const SkPaint* paint,
                                    DrawBitmapRectFlags flags) {
    SkDEBUGCODE(bitmap.validate();)
    this->internalDrawBitmapRect(bitmap, src, dst, paint, flags);
}

void SkCanvas::drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& matrix,
                                const SkPaint* paint) {
    SkDEBUGCODE(bitmap.validate();)
    this->internalDrawBitmap(bitmap, matrix, paint);
}

void SkCanvas::internalDrawBitmapNine(const SkBitmap& bitmap,
                                      const SkIRect& center, const SkRect& dst,
                                      const SkPaint* paint) {
    if (bitmap.drawsNothing()) {
        return;
    }
    if (NULL == paint || paint->canComputeFastBounds()) {
        SkRect storage;
        const SkRect* bounds = &dst;
        if (paint) {
            bounds = &paint->computeFastBounds(dst, &storage);
        }
        if (this->quickReject(*bounds)) {
            return;
        }
    }

    const int32_t w = bitmap.width();
    const int32_t h = bitmap.height();

    SkIRect c = center;
    // pin center to the bounds of the bitmap
    c.fLeft = SkMax32(0, center.fLeft);
    c.fTop = SkMax32(0, center.fTop);
    c.fRight = SkPin32(center.fRight, c.fLeft, w);
    c.fBottom = SkPin32(center.fBottom, c.fTop, h);

    const SkScalar srcX[4] = {
        0, SkIntToScalar(c.fLeft), SkIntToScalar(c.fRight), SkIntToScalar(w)
    };
    const SkScalar srcY[4] = {
        0, SkIntToScalar(c.fTop), SkIntToScalar(c.fBottom), SkIntToScalar(h)
    };
    SkScalar dstX[4] = {
        dst.fLeft, dst.fLeft + SkIntToScalar(c.fLeft),
        dst.fRight - SkIntToScalar(w - c.fRight), dst.fRight
    };
    SkScalar dstY[4] = {
        dst.fTop, dst.fTop + SkIntToScalar(c.fTop),
        dst.fBottom - SkIntToScalar(h - c.fBottom), dst.fBottom
    };

    if (dstX[1] > dstX[2]) {
        dstX[1] = dstX[0] + (dstX[3] - dstX[0]) * c.fLeft / (w - c.width());
        dstX[2] = dstX[1];
    }

    if (dstY[1] > dstY[2]) {
        dstY[1] = dstY[0] + (dstY[3] - dstY[0]) * c.fTop / (h - c.height());
        dstY[2] = dstY[1];
    }

    for (int y = 0; y < 3; y++) {
        SkRect s, d;

        s.fTop = srcY[y];
        s.fBottom = srcY[y+1];
        d.fTop = dstY[y];
        d.fBottom = dstY[y+1];
        for (int x = 0; x < 3; x++) {
            s.fLeft = srcX[x];
            s.fRight = srcX[x+1];
            d.fLeft = dstX[x];
            d.fRight = dstX[x+1];
            this->internalDrawBitmapRect(bitmap, &s, d, paint,
                                         kNone_DrawBitmapRectFlag);
        }
    }
}

void SkCanvas::drawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                              const SkRect& dst, const SkPaint* paint) {
    SkDEBUGCODE(bitmap.validate();)

    // Need a device entry-point, so gpu can use a mesh
    this->internalDrawBitmapNine(bitmap, center, dst, paint);
}

class SkDeviceFilteredPaint {
public:
    SkDeviceFilteredPaint(SkBaseDevice* device, const SkPaint& paint) {
        SkBaseDevice::TextFlags flags;
        if (device->filterTextFlags(paint, &flags)) {
            SkPaint* newPaint = fLazy.set(paint);
            newPaint->setFlags(flags.fFlags);
            fPaint = newPaint;
        } else {
            fPaint = &paint;
        }
    }

    const SkPaint& paint() const { return *fPaint; }

private:
    const SkPaint*  fPaint;
    SkLazyPaint     fLazy;
};

void SkCanvas::DrawRect(const SkDraw& draw, const SkPaint& paint,
                        const SkRect& r, SkScalar textSize) {
    if (paint.getStyle() == SkPaint::kFill_Style) {
        draw.fDevice->drawRect(draw, r, paint);
    } else {
        SkPaint p(paint);
        p.setStrokeWidth(SkScalarMul(textSize, paint.getStrokeWidth()));
        draw.fDevice->drawRect(draw, r, p);
    }
}

void SkCanvas::DrawTextDecorations(const SkDraw& draw, const SkPaint& paint,
                                   const char text[], size_t byteLength,
                                   SkScalar x, SkScalar y) {
    SkASSERT(byteLength == 0 || text != NULL);

    // nothing to draw
    if (text == NULL || byteLength == 0 ||
        draw.fClip->isEmpty() ||
        (paint.getAlpha() == 0 && paint.getXfermode() == NULL)) {
        return;
    }

    SkScalar    width = 0;
    SkPoint     start;

    start.set(0, 0);    // to avoid warning
    if (paint.getFlags() & (SkPaint::kUnderlineText_Flag |
                            SkPaint::kStrikeThruText_Flag)) {
        width = paint.measureText(text, byteLength);

        SkScalar offsetX = 0;
        if (paint.getTextAlign() == SkPaint::kCenter_Align) {
            offsetX = SkScalarHalf(width);
        } else if (paint.getTextAlign() == SkPaint::kRight_Align) {
            offsetX = width;
        }
        start.set(x - offsetX, y);
    }

    if (0 == width) {
        return;
    }

    uint32_t flags = paint.getFlags();

    if (flags & (SkPaint::kUnderlineText_Flag |
                 SkPaint::kStrikeThruText_Flag)) {
        SkScalar textSize = paint.getTextSize();
        SkScalar height = SkScalarMul(textSize, kStdUnderline_Thickness);
        SkRect   r;

        r.fLeft = start.fX;
        r.fRight = start.fX + width;

        if (flags & SkPaint::kUnderlineText_Flag) {
            SkScalar offset = SkScalarMulAdd(textSize, kStdUnderline_Offset,
                                             start.fY);
            r.fTop = offset;
            r.fBottom = offset + height;
            DrawRect(draw, paint, r, textSize);
        }
        if (flags & SkPaint::kStrikeThruText_Flag) {
            SkScalar offset = SkScalarMulAdd(textSize, kStdStrikeThru_Offset,
                                             start.fY);
            r.fTop = offset;
            r.fBottom = offset + height;
            DrawRect(draw, paint, r, textSize);
        }
    }
}

void SkCanvas::onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                          const SkPaint& paint) {
    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, NULL)

    while (iter.next()) {
        SkDeviceFilteredPaint dfp(iter.fDevice, looper.paint());
        iter.fDevice->drawText(iter, text, byteLength, x, y, dfp.paint());
        DrawTextDecorations(iter, dfp.paint(),
                            static_cast<const char*>(text), byteLength, x, y);
    }

    LOOPER_END
}

void SkCanvas::onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                             const SkPaint& paint) {
    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, NULL)

    while (iter.next()) {
        SkDeviceFilteredPaint dfp(iter.fDevice, looper.paint());
        iter.fDevice->drawPosText(iter, text, byteLength, &pos->fX, 0, 2,
                                  dfp.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                              SkScalar constY, const SkPaint& paint) {
    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, NULL)

    while (iter.next()) {
        SkDeviceFilteredPaint dfp(iter.fDevice, looper.paint());
        iter.fDevice->drawPosText(iter, text, byteLength, xpos, constY, 1,
                                  dfp.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                const SkMatrix* matrix, const SkPaint& paint) {
    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, NULL)

    while (iter.next()) {
        iter.fDevice->drawTextOnPath(iter, text, byteLength, path,
                                     matrix, looper.paint());
    }

    LOOPER_END
}

void SkCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                              const SkPaint& paint) {

    // FIXME: temporarily disable quickreject for empty bounds,
    // pending implicit blob bounds implementation.
    if (!blob->bounds().isEmpty() && paint.canComputeFastBounds()) {
        SkRect storage;

        if (this->quickReject(paint.computeFastBounds(blob->bounds().makeOffset(x, y), &storage))) {
            return;
        }
    }

    LOOPER_BEGIN(paint, SkDrawFilter::kText_Type, NULL)

    while (iter.next()) {
        SkDeviceFilteredPaint dfp(iter.fDevice, looper.paint());
        iter.fDevice->drawTextBlob(iter, blob, x, y, dfp.paint());
    }

    LOOPER_END
}

static const int  kLocalSize = 100;
FunFontFallback SkCanvas::s_fontFallback = NULL;

void SkCanvas::SetFontFallback(FunFontFallback fun)
{
	s_fontFallback = fun;
}

// These will become non-virtual, so they always call the (virtual) onDraw... method
void SkCanvas::drawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,const SkPaint& paint) {
	SkTypeface *pfont = paint.getTypeface();
    SkASSERT(pfont);
	pfont->ref();
	uint16_t  glyphs_[kLocalSize];
	size_t length = byteLength/sizeof(wchar_t);
	const wchar_t *wText = (const wchar_t*)text;
	uint16_t* glyphs = length<=kLocalSize ? glyphs_: (new uint16_t[length]);
#if WCHAR_SIZE==4
	int nValids=pfont->charsToGlyphs(wText,SkTypeface::kUTF32_Encoding,glyphs,(int)length);
#else
    int nValids = pfont->charsToGlyphs(wText, SkTypeface::kUTF16_Encoding, glyphs, (int)length);
#endif
    if(nValids==length || !s_fontFallback)
	{
		onDrawText(text,byteLength,x,y,paint);
	}else
	{
		SkPaint *pPaint = (SkPaint*)&paint;
		const wchar_t*pEnd = wText+length;
		int iWord = 0;
		while(wText<pEnd){
			SkTypeface * font2=NULL;
            const wchar_t*pWord = wText;
#if WCHAR_SIZE==4
            wText++;
#else
            SkUTF16_NextUnichar((const uint16_t**)&wText);
#endif
            size_t sizeofutfchar = (wText-pWord)*sizeof(wchar_t);
			if(glyphs[iWord]==0)
			{
				font2 = s_fontFallback(pfont,(const wchar_t*)pWord,(wText-pWord));
				if(font2)
					pPaint->setTypeface(font2);
			}
            onDrawText(pWord, sizeofutfchar, x, y, paint);
            x += paint.measureText(pWord, sizeofutfchar);
			if(font2)
			{
				pPaint->setTypeface(pfont);
				font2->unref();
            }
			iWord++;
		}
	}
	pfont->unref();
	if(length>kLocalSize)
		delete []glyphs;
}

void SkCanvas::drawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                           const SkPaint& paint) {
    this->onDrawPosText(text, byteLength, pos, paint);
}
void SkCanvas::drawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                            SkScalar constY, const SkPaint& paint) {
    this->onDrawPosTextH(text, byteLength, xpos, constY, paint);
}
void SkCanvas::drawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                              const SkMatrix* matrix, const SkPaint& paint) {
    this->onDrawTextOnPath(text, byteLength, path, matrix, paint);
}
void SkCanvas::drawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                            const SkPaint& paint) {
    if (blob) {
        this->onDrawTextBlob(blob, x, y, paint);
    }
}

void SkCanvas::drawVertices(VertexMode vmode, int vertexCount,
                            const SkPoint verts[], const SkPoint texs[],
                            const SkColor colors[], SkXfermode* xmode,
                            const uint16_t indices[], int indexCount,
                            const SkPaint& paint) {
    LOOPER_BEGIN(paint, SkDrawFilter::kPath_Type, NULL)

    while (iter.next()) {
        iter.fDevice->drawVertices(iter, vmode, vertexCount, verts, texs,
                                   colors, xmode, indices, indexCount,
                                   looper.paint());
    }

    LOOPER_END
}

void SkCanvas::drawPatch(const SkPoint cubics[12], const SkColor colors[4],
                         const SkPoint texCoords[4], SkXfermode* xmode, const SkPaint& paint) {
    if (NULL == cubics) {
        return;
    }

    // Since a patch is always within the convex hull of the control points, we discard it when its
    // bounding rectangle is completely outside the current clip.
    SkRect bounds;
    bounds.set(cubics, SkPatchUtils::kNumCtrlPts);
    if (this->quickReject(bounds)) {
        return;
    }

    this->onDrawPatch(cubics, colors, texCoords, xmode, paint);
}

void SkCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                           const SkPoint texCoords[4], SkXfermode* xmode, const SkPaint& paint) {

    LOOPER_BEGIN(paint, SkDrawFilter::kPath_Type, NULL)

    while (iter.next()) {
        iter.fDevice->drawPatch(iter, cubics, colors, texCoords, xmode, paint);
    }

    LOOPER_END
}

//////////////////////////////////////////////////////////////////////////////
// These methods are NOT virtual, and therefore must call back into virtual
// methods, rather than actually drawing themselves.
//////////////////////////////////////////////////////////////////////////////

void SkCanvas::drawARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b,
                        SkXfermode::Mode mode) {
    SkPaint paint;

    paint.setARGB(a, r, g, b);
    if (SkXfermode::kSrcOver_Mode != mode) {
        paint.setXfermodeMode(mode);
    }
    this->drawPaint(paint);
}

void SkCanvas::drawColor(SkColor c, SkXfermode::Mode mode) {
    SkPaint paint;

    paint.setColor(c);
    if (SkXfermode::kSrcOver_Mode != mode) {
        paint.setXfermodeMode(mode);
    }
    this->drawPaint(paint);
}

void SkCanvas::drawPoint(SkScalar x, SkScalar y, const SkPaint& paint) {
    SkPoint pt;

    pt.set(x, y);
    this->drawPoints(kPoints_PointMode, 1, &pt, paint);
}

void SkCanvas::drawPoint(SkScalar x, SkScalar y, SkColor color) {
    SkPoint pt;
    SkPaint paint;

    pt.set(x, y);
    paint.setColor(color);
    this->drawPoints(kPoints_PointMode, 1, &pt, paint);
}

void SkCanvas::drawLine(SkScalar x0, SkScalar y0, SkScalar x1, SkScalar y1,
                        const SkPaint& paint) {
    SkPoint pts[2];

    pts[0].set(x0, y0);
    pts[1].set(x1, y1);
    this->drawPoints(kLines_PointMode, 2, pts, paint);
}

void SkCanvas::drawRectCoords(SkScalar left, SkScalar top,
                              SkScalar right, SkScalar bottom,
                              const SkPaint& paint) {
    SkRect  r;

    r.set(left, top, right, bottom);
    this->drawRect(r, paint);
}

void SkCanvas::drawCircle(SkScalar cx, SkScalar cy, SkScalar radius,
                          const SkPaint& paint) {
    if (radius < 0) {
        radius = 0;
    }

    SkRect  r;
    r.set(cx - radius, cy - radius, cx + radius, cy + radius);
    this->drawOval(r, paint);
}

void SkCanvas::drawRoundRect(const SkRect& r, SkScalar rx, SkScalar ry,
                             const SkPaint& paint) {
    if (rx > 0 && ry > 0) {
        if (paint.canComputeFastBounds()) {
            SkRect storage;
            if (this->quickReject(paint.computeFastBounds(r, &storage))) {
                return;
            }
        }
        SkRRect rrect;
        rrect.setRectXY(r, rx, ry);
        this->drawRRect(rrect, paint);
    } else {
        this->drawRect(r, paint);
    }
}

void SkCanvas::drawArc(const SkRect& oval, SkScalar startAngle,
                       SkScalar sweepAngle, bool useCenter,
                       const SkPaint& paint) {
    if (SkScalarAbs(sweepAngle) >= SkIntToScalar(360)) {
        this->drawOval(oval, paint);
    } else {
        SkPath  path;
        if (useCenter) {
            path.moveTo(oval.centerX(), oval.centerY());
        }
        path.arcTo(oval, startAngle, sweepAngle, !useCenter);
        if (useCenter) {
            path.close();
        }
        this->drawPath(path, paint);
    }
}

void SkCanvas::drawTextOnPathHV(const void* text, size_t byteLength,
                                const SkPath& path, SkScalar hOffset,
                                SkScalar vOffset, const SkPaint& paint) {
    SkMatrix    matrix;

    matrix.setTranslate(hOffset, vOffset);
    this->drawTextOnPath(text, byteLength, path, &matrix, paint);
}

///////////////////////////////////////////////////////////////////////////////
void SkCanvas::EXPERIMENTAL_optimize(const SkPicture* picture) {
    SkBaseDevice* device = this->getDevice();
    if (device) {
        device->EXPERIMENTAL_optimize(picture);
    }
}

void SkCanvas::drawPicture(const SkPicture* picture) {
    if (picture) {
        this->onDrawPicture(picture, NULL, NULL);
    }
}

void SkCanvas::drawPicture(const SkPicture* picture, const SkMatrix* matrix, const SkPaint* paint) {
    if (picture) {
        if (matrix && matrix->isIdentity()) {
            matrix = NULL;
        }
        this->onDrawPicture(picture, matrix, paint);
    }
}

void SkCanvas::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                             const SkPaint* paint) {
    SkBaseDevice* device = this->getTopDevice();
    if (device) {
        // Canvas has to first give the device the opportunity to render
        // the picture itself.
        if (device->EXPERIMENTAL_drawPicture(this, picture, matrix, paint)) {
            return; // the device has rendered the entire picture
        }
    }

    SkAutoCanvasMatrixPaint acmp(this, matrix, paint, picture->cullRect());

    picture->playback(this);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SkCanvas::LayerIter::LayerIter(SkCanvas* canvas, bool skipEmptyClips) {
    SK_COMPILE_ASSERT(sizeof(fStorage) >= sizeof(SkDrawIter), fStorage_too_small);

    SkASSERT(canvas);

    fImpl = new (fStorage) SkDrawIter(canvas, skipEmptyClips);
    fDone = !fImpl->next();
}

SkCanvas::LayerIter::~LayerIter() {
    fImpl->~SkDrawIter();
}

void SkCanvas::LayerIter::next() {
    fDone = !fImpl->next();
}

SkBaseDevice* SkCanvas::LayerIter::device() const {
    return fImpl->getDevice();
}

const SkMatrix& SkCanvas::LayerIter::matrix() const {
    return fImpl->getMatrix();
}

const SkPaint& SkCanvas::LayerIter::paint() const {
    const SkPaint* paint = fImpl->getPaint();
    if (NULL == paint) {
        paint = &fDefaultPaint;
    }
    return *paint;
}

const SkRegion& SkCanvas::LayerIter::clip() const { return fImpl->getClip(); }
int SkCanvas::LayerIter::x() const { return fImpl->getX(); }
int SkCanvas::LayerIter::y() const { return fImpl->getY(); }

///////////////////////////////////////////////////////////////////////////////

SkCanvasClipVisitor::~SkCanvasClipVisitor() { }

///////////////////////////////////////////////////////////////////////////////

static bool supported_for_raster_canvas(const SkImageInfo& info) {
    switch (info.alphaType()) {
        case kPremul_SkAlphaType:
        case kOpaque_SkAlphaType:
            break;
        default:
            return false;
    }

    switch (info.colorType()) {
        case kAlpha_8_SkColorType:
        case kRGB_565_SkColorType:
        case kN32_SkColorType:
            break;
        default:
            return false;
    }

    return true;
}

SkCanvas* SkCanvas::NewRaster(const SkImageInfo& info) {
    if (!supported_for_raster_canvas(info)) {
        return NULL;
    }

    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(info)) {
        return NULL;
    }

    // should this functionality be moved into allocPixels()?
    if (!bitmap.info().isOpaque()) {
        bitmap.eraseColor(0);
    }
    return SkNEW_ARGS(SkCanvas, (bitmap));
}

SkCanvas* SkCanvas::NewRasterDirect(const SkImageInfo& info, void* pixels, size_t rowBytes) {
    if (!supported_for_raster_canvas(info)) {
        return NULL;
    }

    SkBitmap bitmap;
    if (!bitmap.installPixels(info, pixels, rowBytes)) {
        return NULL;
    }
    return SkNEW_ARGS(SkCanvas, (bitmap));
}

///////////////////////////////////////////////////////////////////////////////

SkAutoCanvasMatrixPaint::SkAutoCanvasMatrixPaint(SkCanvas* canvas, const SkMatrix* matrix,
                                                 const SkPaint* paint, const SkRect& bounds)
    : fCanvas(canvas)
    , fSaveCount(canvas->getSaveCount())
{
    if (paint) {
        SkRect newBounds = bounds;
        if (matrix) {
            matrix->mapRect(&newBounds);
        }
        canvas->saveLayer(&newBounds, paint);
    } else if (matrix) {
        canvas->save();
    }

    if (matrix) {
        canvas->concat(*matrix);
    }
}

SkAutoCanvasMatrixPaint::~SkAutoCanvasMatrixPaint() {
    fCanvas->restoreToCount(fSaveCount);
}
