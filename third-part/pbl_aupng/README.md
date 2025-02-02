# pbl_aupng - [upng](https://github.com/elanthis/upng) with [APNG](https://wiki.mozilla.org/APNG_Specification) support for pebble

## Why?

upng is currently used by [RebbleOS](https://github.com/ginge/FreeRTOS-Pebble) which strives to be API-compatible to the old PebbleOS including `gbitmap_sequence`, which uses APNG images. Thus this project is for developing the basic APNG support and then implement the `gbitmap_sequence` API.
In the process much of the vast support of the original upng is removed in order to suit pebble. In turn this project might be very unsuitable for everything non-pebble.

## How?

You will need cmake to build this project. Use these commands to configure, build and run the tests:

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
ctest .
```

## Problems?

Yes.

### Dispose Operation "Previous"

One of the operations to be executed on the buffer before blending over the next frame is "previous", which means the area of the current frame (in the buffer) shall be restored with the data present before the current frame. In short every frame may be dependant on the previous previous frame. Oh nay...

So without having to redecode the previous previous frame (which might be dependant on yet more previous frames) aupng has two long-living buffers, the `frameBuffer` in which the currently decoded frame resides and if needed the `previousBuffer` in which the previous to current decoded frame is stored. In pseudocode the disposing of a frame might look like this:

```
if is first frame
    clear frameBuffer
else
    if previousframe->dispose_op = background
        clear previousframe->area in frameBuffer
    else if previousframe->dispose_op = previous
        copy previousBuffer to frameBuffer
        free previousBuffer

    if currentframe->dispose_op = previous
        copy currentframe->area of frameBuffer in previousBuffer

blend frame over frameBuffer
```

### Blend Operation "Over"

One of the operations that tell how to blend frames over another is the "over" operation which is your standard alpha blending, the PNG reference states the formula:

`output = alpha * foreground + (1-alpha) * background`

Now lets see how this actually turns out for different color types in aupng:

| Color Type | What do we have to do? |
|:----------:|:-----------------------|
| RGB8 <br/> RGB16 <br/> Luminance1 <br/> Luminance2 <br/> Luminance4 <br/> Luminance8 | Great! No alpha means full alpha, so this is the same as copy |
| RGBA8 <br/> RGBA16 <br/> LuminanceAlpha1 <br/> LuminanceAlpha2 <br/> LuminanceAlpha4 <br/> LuminanceAlpha8 | Now we do have alpha information so we actually have to alpha blend |
| Indexed1 <br/> Indexed2 <br/> Indexed4 <br/> Indexed8 <br/> | Oh did you notice? None of the formats above are (often) used for Pebble Images! Almost all of them are palettized images and they make the most problems, here is why: |

Now step back, there is an "easy" way out, due to the first frame always being the full size of the png and the precondition that all entries in the alpha palette are full or if there is no tRNS (alpha palette) chunk altogether, then every over blend operation is the same as a copy blend operation.

But if there are partial-alpha entries there might be the case that there aren't the colors calculated in the alpha blending in the palette to represent them in the frameBuffer! So now we have to convert the color type in order to support the blend operation. But to what color type? RGBA16 is the one where you would have the least error, RGBA8 is the one most useful to every-day applications and RGBA2, which isn't even a PNG color format, is the only one acceptable for the Pebble...
