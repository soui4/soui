/* 把 cmp_shot.cjs 抓下来的两套逐标签截图拼成并排对照图，并打印像素差。
 *
 * 用法：node tools/diag/cmp_make.cjs [cairo 目录] [canvas2d 目录]
 * 产物：build/cmp/pair_NN.png（左 cairo / 右 canvas2d）、build/cmp/pair_NN_diff.png
 */
'use strict';
const fs = require('fs');
const path = require('path');
const { decodePNG, encodePNG, newImg, blit, diff } = require('./img.cjs');

const ROOT = path.join(__dirname, '..', '..', 'build', 'cmp');
const A_DIR = process.argv[2] || path.join(ROOT, 'cairo');
const B_DIR = process.argv[3] || path.join(ROOT, 'canvas2d');

const names = ['概览', '基础控件', '更多控件', 'MVC', '列表与树', '布局', '图形控件', '表格', '图表', '编辑器', '扩展控件', '动画', 'Lua 游戏', '模态表单'];

const rows = [];
for (let i = 0; i < 20; i++) {
  const nn = String(i).padStart(2, '0');
  const fa = path.join(A_DIR, nn + '.png');
  const fb = path.join(B_DIR, nn + '.png');
  if (!fs.existsSync(fa) || !fs.existsSync(fb)) break;
  const A = decodePNG(fs.readFileSync(fa));
  const B = decodePNG(fs.readFileSync(fb));
  const D = diff(A, B);
  const img = newImg(A.w + B.w + 8, Math.max(A.h, B.h), [8, 8, 8, 255]);
  blit(img, A, 0, 0);
  blit(img, B, A.w + 8, 0);
  fs.writeFileSync(path.join(ROOT, 'pair_' + nn + '.png'), encodePNG(img));
  fs.writeFileSync(path.join(ROOT, 'pair_' + nn + '_diff.png'), encodePNG(D.map));
  rows.push({ i: nn, name: names[i] || '?', pct: D.pct, px: D.diff });
}

rows.sort((a, b) => b.pct - a.pct);
console.log('标签          差异%    差异像素');
for (const r of rows) {
  console.log(
    (r.i + ' ' + r.name).padEnd(14, ' ') + String(r.pct.toFixed(2)).padStart(6) + '%  ' + String(r.px).padStart(7)
  );
}
console.log('\n并排图：build/cmp/pair_NN.png（左 cairo，右 canvas2d）');
