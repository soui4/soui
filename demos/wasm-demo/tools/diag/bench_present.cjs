/* 量化 canvas2d 后端 present 路径各步骤的成本：
 *   getImageData 全屏读回 / Uint32 逐像素 swap / putImageData 全屏上传
 * 并对比 willReadFrequently 开关的差异。纯浏览器侧，不依赖 wasm。 */
const { chromium } = require('playwright-core');

(async () => {
  const browser = await chromium.launch({ headless: true, channel: 'msedge' });
  const page = await browser.newPage({ viewport: { width: 1400, height: 1000 } });
  await page.goto('about:blank');

  const r = await page.evaluate(() => {
    const W = 1200, H = 900;
    const N = 40; // 重复次数

    function mkCanvas(opts) {
      const c = document.createElement('canvas');
      c.width = W; c.height = H;
      document.body.appendChild(c);
      const ctx = c.getContext('2d', opts);
      // 画点内容，避免全透明快路径
      ctx.fillStyle = '#f0f0f0';
      ctx.fillRect(0, 0, W, H);
      for (let i = 0; i < 200; ++i) {
        ctx.fillStyle = `hsl(${(i * 37) % 360},70%,50%)`;
        ctx.fillRect((i * 97) % W, (i * 53) % H, 120, 80);
      }
      ctx.fillStyle = '#123456';
      ctx.font = '20px sans-serif';
      for (let i = 0; i < 60; ++i) ctx.fillText('SOUI canvas2d 性能测试 ' + i, 20, 20 + i * 14);
      return { c, ctx };
    }

    function bench(label, opts) {
      const { c, ctx } = mkCanvas(opts);
      // 目标缓冲（模拟 fb）与 ImageData 图像
      const img = ctx.createImageData(W, H);
      const img32 = new Uint32Array(img.data.buffer);
      const fb = new Uint32Array(W * H);

      // 1) getImageData
      let t0 = performance.now();
      let got;
      for (let i = 0; i < N; ++i) got = ctx.getImageData(0, 0, W, H);
      const tGet = (performance.now() - t0) / N;

      // 2) typed array set（把 RGBA 拷进 wasm 堆的等价操作）
      t0 = performance.now();
      for (let i = 0; i < N; ++i) fb.set(new Uint32Array(got.data.buffer));
      const tSet = (performance.now() - t0) / N;

      // 3) 逐像素 ARGB<->RGBA swap（C++ 侧和 JS 上屏各做一次的等价物）
      t0 = performance.now();
      for (let i = 0; i < N; ++i) {
        for (let p = 0; p < W * H; ++p) {
          const v = fb[p];
          img32[p] = (v & 0xFF00FF00) | ((v & 0xFF) << 16) | ((v >> 16) & 0xFF);
        }
      }
      const tSwap = (performance.now() - t0) / N;

      // 4) putImageData 全屏
      t0 = performance.now();
      for (let i = 0; i < N; ++i) ctx.putImageData(img, 0, 0);
      const tPut = (performance.now() - t0) / N;

      // 5) drawImage 全屏（GPU 合成路径的对照）
      const c2 = document.createElement('canvas');
      c2.width = W; c2.height = H;
      const ctx2 = c2.getContext('2d');
      t0 = performance.now();
      for (let i = 0; i < N; ++i) ctx2.drawImage(c, 0, 0);
      const tDraw = (performance.now() - t0) / N;

      c.remove(); c2.remove();
      return {
        label,
        getImageData: +tGet.toFixed(2),
        typSet: +tSet.toFixed(2),
        swapLoop: +tSwap.toFixed(2),
        putImageData: +tPut.toFixed(2),
        drawImage: +tDraw.toFixed(2),
        total_canvas2d: +(tGet + tSet + tSwap * 2 + tPut).toFixed(2),
        total_cairo: +(tSet + tSwap + tPut).toFixed(2),
      };
    }

    return [
      bench('default(alpha:false)', { alpha: false }),
      bench('willReadFrequently', { alpha: false, willReadFrequently: true }),
    ];
  });

  console.log('canvas 1200x900，单位 ms/帧：');
  for (const row of r) console.log(JSON.stringify(row));
  await browser.close();
})();
