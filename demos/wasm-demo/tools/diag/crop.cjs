/* 裁剪 + 放大 + 可选两图并排，用于逐像素看局部（九宫格接缝、边框颜色这类）。
 *
 * 用法：
 *   node tools/diag/crop.cjs <a.png> <x> <y> <w> <h> [out.png] [scale]
 *   node tools/diag/crop.cjs <a.png> <b.png> <x> <y> <w> <h> [out.png] [scale]
 */
'use strict';
const fs = require('fs');
const path = require('path');
const { decodePNG, encodePNG, newImg, blit, scale } = require('./img.cjs');

function crop(img, x, y, w, h) {
  const out = newImg(w, h, [0, 0, 0, 255]);
  for (let yy = 0; yy < h; yy++) {
    for (let xx = 0; xx < w; xx++) {
      const sx = x + xx,
        sy = y + yy;
      const di = (yy * w + xx) * 4;
      if (sx < 0 || sy < 0 || sx >= img.w || sy >= img.h) continue;
      const si = (sy * img.w + sx) * 4;
      out.data[di] = img.data[si];
      out.data[di + 1] = img.data[si + 1];
      out.data[di + 2] = img.data[si + 2];
      out.data[di + 3] = 255;
    }
  }
  return out;
}

const args = process.argv.slice(2);
const isTwo = args[0] && args[0].toLowerCase().endsWith('.png') && args[1] && args[1].toLowerCase().endsWith('.png');
const nums = (isTwo ? args.slice(2) : args.slice(1)).map((v) => parseFloat(v));
const files = isTwo ? [args[0], args[1]] : [args[0]];
const [x, y, w, h] = nums.slice(0, 4);
const out = isTwo ? path.join(path.dirname(files[0]), 'crop_pair.png') : 'crop.png';
const zoom = 3;

const imgs = files.map((f) => decodePNG(fs.readFileSync(f)));
const parts = imgs.map((im) => crop(im, x, y, w, h));
let canvas;
if (parts.length === 2) {
  canvas = newImg(w * 2 + 6, h, [255, 0, 255, 255]);
  blit(canvas, parts[0], 0, 0);
  canvas.data.fill(255, 0, 0, 0); /* noop 防优化 */
  blit(canvas, parts[1], w + 6, 0);
  for (let yy = 0; yy < h; yy++)
    for (let xx = 0; xx < 6; xx++) {
      const i = (yy * canvas.w + w + xx) * 4;
      canvas.data[i] = 255;
      canvas.data[i + 1] = 0;
      canvas.data[i + 2] = 255;
    }
} else canvas = parts[0];

const big = scale(canvas, zoom);
const target = isTwo ? path.join(path.dirname(files[0]), 'crop_pair.png') : out;
fs.writeFileSync(target, encodePNG(big));
console.log('WROTE ' + target + ' ' + big.w + 'x' + big.h);

/* 顺带把两图在该区域内的像素差打印出来，便于确认是"整体色差"还是"结构缺失" */
if (isTwo) {
  const A = imgs[0],
    B = imgs[1];
  let n = 0;
  const hist = new Map();
  for (let yy = y; yy < y + h; yy++)
    for (let xx = x; xx < x + w; xx++) {
      const si = (yy * A.w + xx) * 4;
      const d =
        Math.abs(A.data[si] - B.data[si]) + Math.abs(A.data[si + 1] - B.data[si + 1]) + Math.abs(A.data[si + 2] - B.data[si + 2]);
      if (d > 30) n++;
      const k = A.data[si] + ',' + A.data[si + 1] + ',' + A.data[si + 2] + ' -> ' + B.data[si] + ',' + B.data[si + 1] + ',' + B.data[si + 2];
      hist.set(k, (hist.get(k) || 0) + 1);
    }
  console.log('区域差异像素 ' + n + '/' + w * h);
  const top = [...hist.entries()].sort((a, b) => b[1] - a[1]).slice(0, 10);
  for (const [k, v] of top) console.log('  ' + v + 'x  ' + k);
}
