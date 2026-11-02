/* 无依赖 PNG 读写 + 并排合成 + 像素差统计。
 * 本机没有 PIL/numpy，但 Node 自带 zlib，解析 PNG 只差一个 un-filter 循环。
 *
 * 用法（一般由 cmp_make.cjs 调用，也可单独跑）：
 *   node tools/diag/img.cjs <a.png> <b.png> <out.png>
 */
'use strict';
const fs = require('fs');
const zlib = require('zlib');

const PNG_SIG = Buffer.from([0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a]);

function decodePNG(buf) {
  if (buf.slice(0, 8).compare(PNG_SIG) !== 0) throw new Error('不是 PNG');
  let off = 8;
  let w = 0,
    h = 0,
    bitDepth = 8,
    colorType = 6,
    interlace = 0;
  const idat = [];
  let plte = null,
    trns = null;
  while (off < buf.length) {
    const len = buf.readUInt32BE(off);
    const type = buf.toString('ascii', off + 4, off + 8);
    const data = buf.slice(off + 8, off + 8 + len);
    off += 12 + len;
    if (type === 'IHDR') {
      w = data.readUInt32BE(0);
      h = data.readUInt32BE(4);
      bitDepth = data[8];
      colorType = data[9];
      interlace = data[12];
    } else if (type === 'PLTE') plte = data;
    else if (type === 'tRNS') trns = data;
    else if (type === 'IDAT') idat.push(data);
    else if (type === 'IEND') break;
  }
  if (bitDepth !== 8) throw new Error('只支持 8bit，实际 ' + bitDepth);
  if (interlace !== 0) throw new Error('不支持隔行 PNG');

  const raw = zlib.inflateSync(Buffer.concat(idat));
  const channels = { 0: 1, 2: 3, 3: 1, 4: 2, 6: 4 }[colorType];
  if (!channels) throw new Error('不支持的 colorType ' + colorType);
  const stride = w * channels;
  const out = Buffer.alloc(w * h * 4);
  const prev = Buffer.alloc(stride);
  const cur = Buffer.alloc(stride);
  let p = 0;
  for (let y = 0; y < h; y++) {
    const ft = raw[p++];
    raw.copy(cur, 0, p, p + stride);
    p += stride;
    for (let i = 0; i < stride; i++) {
      const a = i >= channels ? cur[i - channels] : 0;
      const b = prev[i];
      const c = i >= channels ? prev[i - channels] : 0;
      let v = cur[i];
      switch (ft) {
        case 0:
          break;
        case 1:
          v = (v + a) & 0xff;
          break;
        case 2:
          v = (v + b) & 0xff;
          break;
        case 3:
          v = (v + ((a + b) >> 1)) & 0xff;
          break;
        case 4: {
          const pp = a + b - c;
          const pa = Math.abs(pp - a),
            pb = Math.abs(pp - b),
            pc = Math.abs(pp - c);
          v = (v + (pa <= pb && pa <= pc ? a : pb <= pc ? b : c)) & 0xff;
          break;
        }
        default:
          throw new Error('未知 filter ' + ft);
      }
      cur[i] = v;
    }
    /* 展开到 RGBA */
    const o = y * w * 4;
    for (let x = 0; x < w; x++) {
      let r, g, b2, al = 255;
      if (colorType === 6) {
        r = cur[x * 4];
        g = cur[x * 4 + 1];
        b2 = cur[x * 4 + 2];
        al = cur[x * 4 + 3];
      } else if (colorType === 2) {
        r = cur[x * 3];
        g = cur[x * 3 + 1];
        b2 = cur[x * 3 + 2];
      } else if (colorType === 0) {
        r = g = b2 = cur[x];
      } else if (colorType === 4) {
        r = g = b2 = cur[x * 2];
        al = cur[x * 2 + 1];
      } else {
        const idx = cur[x];
        r = plte[idx * 3];
        g = plte[idx * 3 + 1];
        b2 = plte[idx * 3 + 2];
        if (trns && idx < trns.length) al = trns[idx];
      }
      out[o + x * 4] = r;
      out[o + x * 4 + 1] = g;
      out[o + x * 4 + 2] = b2;
      out[o + x * 4 + 3] = al;
    }
    cur.copy(prev);
  }
  return { w, h, data: out };
}

const CRC_TABLE = (() => {
  const t = new Int32Array(256);
  for (let n = 0; n < 256; n++) {
    let c = n;
    for (let k = 0; k < 8; k++) c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1;
    t[n] = c;
  }
  return t;
})();

function crc32(buf) {
  let c = -1;
  for (let i = 0; i < buf.length; i++) c = CRC_TABLE[(c ^ buf[i]) & 0xff] ^ (c >>> 8);
  return (c ^ -1) >>> 0;
}

function chunk(type, data) {
  const len = Buffer.alloc(4);
  len.writeUInt32BE(data.length, 0);
  const t = Buffer.from(type, 'ascii');
  const crc = Buffer.alloc(4);
  crc.writeUInt32BE(crc32(Buffer.concat([t, data])), 0);
  return Buffer.concat([len, t, data, crc]);
}

function encodePNG(img) {
  const { w, h, data } = img;
  const ihdr = Buffer.alloc(13);
  ihdr.writeUInt32BE(w, 0);
  ihdr.writeUInt32BE(h, 4);
  ihdr[8] = 8;
  ihdr[9] = 6;
  ihdr[10] = 0;
  ihdr[11] = 0;
  ihdr[12] = 0;
  const raw = Buffer.alloc(h * (w * 4 + 1));
  for (let y = 0; y < h; y++) {
    raw[y * (w * 4 + 1)] = 0;
    data.copy(raw, y * (w * 4 + 1) + 1, y * w * 4, (y + 1) * w * 4);
  }
  return Buffer.concat([
    PNG_SIG,
    chunk('IHDR', ihdr),
    chunk('IDAT', zlib.deflateSync(raw, { level: 6 })),
    chunk('IEND', Buffer.alloc(0)),
  ]);
}

function newImg(w, h, rgba) {
  const d = Buffer.alloc(w * h * 4);
  if (rgba) {
    for (let i = 0; i < w * h; i++) {
      d[i * 4] = rgba[0];
      d[i * 4 + 1] = rgba[1];
      d[i * 4 + 2] = rgba[2];
      d[i * 4 + 3] = rgba[3];
    }
  }
  return { w, h, data: d };
}

/* 把 src 贴到 dst 的 (dx,dy) */
function blit(dst, src, dx, dy) {
  for (let y = 0; y < src.h; y++) {
    const ty = dy + y;
    if (ty < 0 || ty >= dst.h) continue;
    for (let x = 0; x < src.w; x++) {
      const tx = dx + x;
      if (tx < 0 || tx >= dst.w) continue;
      const si = (y * src.w + x) * 4;
      const di = (ty * dst.w + tx) * 4;
      dst.data[di] = src.data[si];
      dst.data[di + 1] = src.data[si + 1];
      dst.data[di + 2] = src.data[si + 2];
      dst.data[di + 3] = 255;
    }
  }
}

/* 最近邻缩放，只用于拼图预览 */
function scale(src, f) {
  const w = Math.max(1, Math.round(src.w * f));
  const h = Math.max(1, Math.round(src.h * f));
  const d = Buffer.alloc(w * h * 4);
  for (let y = 0; y < h; y++) {
    const sy = Math.min(src.h - 1, Math.floor(y / f));
    for (let x = 0; x < w; x++) {
      const sx = Math.min(src.w - 1, Math.floor(x / f));
      const si = (sy * src.w + sx) * 4;
      const di = (y * w + x) * 4;
      d[di] = src.data[si];
      d[di + 1] = src.data[si + 1];
      d[di + 2] = src.data[si + 2];
      d[di + 3] = 255;
    }
  }
  return { w, h, data: d };
}

/* 逐像素差：返回 {diff, total, pct, map}
 * 阈值 tol 用来放过抗锯齿/字体的细微差别；map 里差异像素标红、相同像素压灰。 */
function diff(a, b, tol = 24) {
  const w = Math.min(a.w, b.w),
    h = Math.min(a.h, b.h);
  const map = newImg(w, h, [26, 26, 26, 255]);
  let n = 0;
  for (let y = 0; y < h; y++) {
    for (let x = 0; x < w; x++) {
      const i = (y * w + x) * 4;
      const d =
        Math.abs(a.data[i] - b.data[i]) +
        Math.abs(a.data[i + 1] - b.data[i + 1]) +
        Math.abs(a.data[i + 2] - b.data[i + 2]);
      if (d > tol * 3) {
        n++;
        map.data[i] = 255;
        map.data[i + 1] = 40;
        map.data[i + 2] = 40;
      }
    }
  }
  return { diff: n, total: w * h, pct: (100 * n) / (w * h), map };
}

module.exports = { decodePNG, encodePNG, newImg, blit, scale, diff };

if (require.main === module) {
  const [a, b, out] = process.argv.slice(2);
  if (!a || !b || !out) {
    console.log('用法: node img.cjs a.png b.png out.png');
    process.exit(1);
  }
  const A = decodePNG(fs.readFileSync(a));
  const B = decodePNG(fs.readFileSync(b));
  const D = diff(A, B);
  const img = newImg(A.w + B.w + 6, Math.max(A.h, B.h), [10, 10, 10, 255]);
  blit(img, A, 0, 0);
  blit(img, B, A.w + 6, 0);
  fs.writeFileSync(out, encodePNG(img));
  console.log(`A=${A.w}x${A.h} B=${B.w}x${B.h} diff=${D.diff} (${D.pct.toFixed(2)}%)`);
  const dp = out.replace(/\.png$/, '') + '_diff.png';
  fs.writeFileSync(dp, encodePNG(D.map));
  console.log('WROTE ' + out + ', ' + dp);
}
