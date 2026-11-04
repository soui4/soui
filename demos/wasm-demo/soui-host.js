/*!
 * soui-host.js -- SOUI on WebAssembly 的通用浏览器宿主（可复用）
 *
 * 一个 Emscripten MODULARIZE 产物（xxx.js + xxx.wasm [+ xxx.data]）在页面里需要的
 * 全部胶水都在这里：带进度的下载、按需载入胶水（防旧缓存）、实例化前的 import 预检、
 * 帧缓冲上屏（canvas2d 直合成 / RGBA 快路径 / ARGB 换序兜底）、鼠标/键盘/滚轮注入、
 * IME 组合输入代理、剪贴板双向桥、WebAudio 音效桥、崩溃上报、加载遮罩。
 *
 * 页面上怎么写（新程序只要这三步，其余全在 C++ 里）：
 *
 *   <script src="soui-host.js"></script>
 *   <div id="app"></div>
 *   <script>
 *     SouiHost.start({
 *       container: document.getElementById('app'),
 *       wasm: 'myapp.wasm',        // 胶水与 .data 默认由它推导
 *       factory: 'createMyApp',    // emcc -sEXPORT_NAME=...
 *     });
 *   </script>
 *
 * C++ 侧要提供的导出（名字可用 cfg.api 覆盖，见下）：
 *   _SwinxDemoPump / _SwinxDemoMouse / _SwinxDemoKey / _SwinxDemoWheel
 *   _SwinxDemoText / _SwinxDemoCaretPos / _SwinxClipboardSetBrowserText
 *   _malloc / _free / _SwinxFbBits / _SwinxFbWidth / _SwinxFbHeight / _SwinxFbStride
 * 以及 EM_ASM 回调 Module.swinxPresent(l,t,r,b) 与 Module.souiFatal(str)。
 * 这些在 demos/wasm-demo/soui_main.cpp 的「宿主接线」一节里就是完整实现，抄一份即可。
 *
 * 配置项（都可省略）：
 *   container      装画布的容器元素（缺省 document.body）；会给它加 .soui-host
 *   canvas         已有的 canvas 元素或选择器（缺省自己建一个）
 *   glue / wasm / data   三个产物文件名（缺省由 wasm 推导：.wasm → .js / .data）
 *   factory        胶水里的工厂函数名（缺省 createSouiDemo）
 *   width / height 画布的 CSS 显示宽度（缺省取 C++ 的逻辑尺寸，见 ensureSurface）
 *   injectCss      是否注入默认样式（缺省 true；自己有样式可设 false）
 *   loading        是否显示加载遮罩（缺省 true）
 *   log            是否在画布旁建一个错误行（缺省 true）
 *   ime            是否启用 IME 组合输入代理（缺省 true；纯展示程序可关）
 *   audio          是否启用 WebAudio 音效桥（缺省 true）
 *   api            覆盖导出名：{pump, mouse, key, wheel, text, caret, clipboard}
 *   loadingTitle   遮罩上的标题
 *   onFatal / onLog  启动失败 / 普通日志的自定义处理（缺省写页面 + 控制台）
 *
 * 返回值：{ canvas, fail, whenReady(Promise), options }；whenReady 在 C++ main()
 * 跑完、第一帧上屏后 resolve。
 */
(function (global) {
  'use strict';

  var VERSION = '1.0.0';

  /* --------------------------------------------------------------------- */
  /* 默认样式：只负责"画面 + 遮罩 + IME 代理 + 错误行"，页面本身的排版/配色不管   */
  /* --------------------------------------------------------------------- */
  var STYLE_ID = 'soui-host-style';
  var CSS = [
    '.soui-screen-wrap{position:relative;display:inline-block;max-width:100%;}',
    /* 显示宽度用 CSS 变量给：逻辑宽度要等 C++ 起来才知道（见 ensureSurface），
     * 直接写 style.width 会跟 max-width 打架，变量不会。 */
    'canvas.soui-canvas{display:block;width:var(--soui-logical-w,800px);max-width:100%;',
    'height:auto;background:#000;border:1px solid #2b3a4d;border-radius:4px;',
    'outline:none;touch-action:none;}',
    'canvas.soui-canvas.keyed{border-color:#4ea1ff;}',
    '.soui-loading{position:absolute;inset:0;display:flex;flex-direction:column;',
    'align-items:center;justify-content:center;gap:10px;padding:0 48px;',
    'background:rgba(13,17,23,.96);border-radius:4px;text-align:center;',
    'transition:opacity .3s ease;}',
    '.soui-loading.done{opacity:0;pointer-events:none;}',
    '.soui-load-title{font-size:15px;font-weight:600;color:#d6e2f0;}',
    '.soui-load-phase{font-size:13px;color:#8fa3b8;}',
    '.soui-loading.failed .soui-load-phase{color:#ff8a7a;}',
    '.soui-load-bar{width:min(420px,80%);height:6px;border-radius:3px;background:#1c2836;overflow:hidden;}',
    '.soui-load-fill{width:0%;height:100%;background:#4ea1ff;',
    'transition:width .15s linear,background-color .2s linear;}',
    '.soui-load-meta{font-size:12px;color:#6f8398;font-variant-numeric:tabular-nums;}',
    /* 输入法代理：canvas 不是可编辑元素，浏览器的输入法只在 input/textarea/
     * contenteditable 上开组合输入。用 opacity 而不是 display:none/width:0 ——
     * 完全隐藏的元素拿不到 IME 上下文，候选窗也不会弹。 */
    'textarea.soui-ime{position:fixed;z-index:5;width:26px;height:22px;margin:0;padding:0;',
    'border:none;outline:none;background:transparent;color:transparent;caret-color:transparent;',
    'font:14px/1.2 sans-serif;resize:none;overflow:hidden;opacity:.02;}',
    '.soui-log{margin-top:10px;color:#ffb08a;min-height:1.4em;}'
  ].join('');

  function injectCss() {
    if (document.getElementById(STYLE_ID)) return;
    var st = document.createElement('style');
    st.id = STYLE_ID;
    st.textContent = CSS;
    document.head.appendChild(st);
  }

  function el(tag, cls, parent, attrs) {
    var e = document.createElement(tag);
    if (cls) e.className = cls;
    if (attrs) for (var k in attrs) e.setAttribute(k, attrs[k]);
    if (parent) parent.appendChild(e);
    return e;
  }

  /* --------------------------------------------------------------------- */
  /* start                                                                  */
  /* --------------------------------------------------------------------- */
  function start(cfg) {
    cfg = cfg || {};

    var API_DEFAULT = {
      pump: '_SwinxDemoPump',
      mouse: '_SwinxDemoMouse',
      key: '_SwinxDemoKey',
      wheel: '_SwinxDemoWheel',
      text: '_SwinxDemoText',
      caret: '_SwinxDemoCaretPos',
      clipboard: '_SwinxClipboardSetBrowserText'
    };
    var api = Object.assign({}, API_DEFAULT, cfg.api || {});

    /* 产物名：给了 wasm 就按它推导，全都不给就按本仓库的默认产物名。 */
    var fWasm = cfg.wasm || 'soui-wasm-demo.wasm';
    var fGlue = cfg.glue || fWasm.replace(/\.wasm$/i, '.js');
    var fData = cfg.data === undefined ? fWasm.replace(/\.wasm$/i, '.data') : cfg.data;
    var factoryName = cfg.factory || 'createSouiDemo';

    var opt = Object.assign(
      {
        container: null,
        canvas: null,
        canvasId: 'screen',   // 自建画布的 id（诊断脚本一律按 #screen 找它）
        width: 0,
        height: 0,
        injectCss: true,
        loading: true,
        log: true,
        ime: true,
        audio: true,
        loadingTitle: 'SOUI on WebAssembly',
        onFatal: null,
        onLog: null
      },
      cfg
    );

    if (opt.injectCss) injectCss();

    /* ---------------- DOM：画布 / 遮罩 / IME 代理 / 错误行 ---------------- */
    var canvas = null;
    if (opt.canvas) {
      canvas = typeof opt.canvas === 'string' ? document.querySelector(opt.canvas) : opt.canvas;
    }
    var container = opt.container
      ? (typeof opt.container === 'string' ? document.querySelector(opt.container) : opt.container)
      : canvas
        ? canvas.parentNode || document.body
        : document.body;

    var wrap = el('div', 'soui-screen-wrap', null, { id: 'screen-wrap' });
    if (!canvas) {
      canvas = el('canvas', 'soui-canvas', null, {
        id: opt.canvasId || 'screen',
        tabindex: '0',
        'aria-label': opt.loadingTitle
      });
      wrap.appendChild(canvas);
    } else {
      canvas.classList.add('soui-canvas');
      wrap.appendChild(canvas);
    }

    var loadEl = null, loadPhaseEl = null, loadFillEl = null, loadMetaEl = null;
    if (opt.loading) {
      loadEl = el('div', 'soui-loading', wrap, { id: 'loading', role: 'status', 'aria-live': 'polite' });
      el('div', 'soui-load-title', loadEl).textContent = opt.loadingTitle;
      loadPhaseEl = el('div', 'soui-load-phase', loadEl, { id: 'load-phase' });
      loadPhaseEl.textContent = '正在加载…';
      var bar = el('div', 'soui-load-bar', loadEl);
      loadFillEl = el('div', 'soui-load-fill', bar, { id: 'load-fill' });
      loadMetaEl = el('div', 'soui-load-meta', loadEl, { id: 'load-meta' });
      loadMetaEl.textContent = '0%';
    }

    var proxy = null;
    if (opt.ime) {
      proxy = el('textarea', 'soui-ime', wrap, {
        id: 'ime-proxy',
        'aria-label': '输入法代理输入框',
        spellcheck: 'false',
        autocomplete: 'off',
        autocorrect: 'off',
        autocapitalize: 'off'
      });
    }

    container.appendChild(wrap);
    var logEl = null;
    if (opt.log) {
      logEl = el('div', 'soui-log', null, { id: 'log' });
      container.appendChild(logEl);
    }

    var ctx = canvas.getContext('2d', { alpha: false });
    /* canvas2d 后端的上屏不走帧缓冲：C++ 侧直接 drawImage 把离屏窗口画布合成到
     * 显示画布，需要这个 ctx（见 swinx js_glue.cpp 的 swx_present_surface）。 */
    global.__swxDisplay = ctx;

    /* ---------------- 运行期状态 ---------------- */
    var ready = false;
    var fb = null;      // {w,h,stride} 帧缓冲物理尺寸
    var img = null;     // 显示画布尺寸的 ImageData（慢路径/窄脏区用）
    var img32 = null;
    var composing = false;   // IME 组合进行中：此时不注入原始按键
    var whenReady = null;
    var resolveReady = null, rejectReady = null;
    if (typeof Promise === 'function') {
      whenReady = new Promise(function (res, rej) { resolveReady = res; rejectReady = rej; });
      /* 没人 await 时不要变成 unhandledrejection（会触发下面的崩溃上报）。 */
      whenReady.catch(function () {});
    }

    /* 失败同时写进页面日志和加载遮罩：启动期的失败（下载、编译、DoConfig）都发生在
     * 画面出来之前，遮罩还盖着，只写日志会被用户当成"一直卡在加载"。 */
    function fail(msg) {
      if (logEl) logEl.textContent = msg;
      console.error(msg);
      if (typeof opt.onFatal === 'function') {
        try { opt.onFatal(msg); } catch (e) {}
      }
      if (loadEl && !loadEl.classList.contains('done')) {
        loadEl.classList.add('failed');
        if (loadPhaseEl) loadPhaseEl.textContent = msg;
        if (loadFillEl) {
          loadFillEl.style.width = '100%';
          loadFillEl.style.backgroundColor = '#ef4444';
        }
        if (loadMetaEl) loadMetaEl.textContent = '';
      }
      if (rejectReady) { rejectReady(new Error(msg)); rejectReady = null; }
    }

    /* ---------------- 音效桥（WebAudio） ---------------- */
    /* 浏览器里唯一可行的发声通道，AudioContext 只能由页面持有，且受 autoplay 策略
     * 约束：必须由用户手势 resume() 成 running 才能真正出声。这里懒创建、在手势里
     * resume、用 onstatechange 冲刷排队请求（绝不在 suspended 上 start）。 */
    var _audioCtx = null;
    var _audioSources = [];   // 正在播放的源，用于 SND_NOSTOP 判重与停止
    var _audioCache = new Map();  // path -> AudioBuffer
    var _audioPending = [];   // {cached,key,loop,noStop} 或 {cached:false,ab,...}

    function _emitAudio(buf, loop, noStop) {
      var ac = _audioCtx;
      if (!ac) return false;
      if (noStop && _audioSources.length > 0) return false; // SND_NOSTOP：不打断正在播的
      var src = ac.createBufferSource();
      src.buffer = buf;
      src.loop = loop;
      src.onended = function () {
        var i = _audioSources.indexOf(src);
        if (i >= 0) _audioSources.splice(i, 1);
      };
      src.connect(ac.destination);
      src.start(0);
      _audioSources.push(src);
      return true;
    }

    function _flushPending() {
      var q = _audioPending;
      _audioPending = [];
      for (var i = 0; i < q.length; ++i) {
        (function (item) {
          if (item.cached) {
            var buf = _audioCache.get(item.key);
            if (buf) _emitAudio(buf, item.loop, item.noStop);
          } else {
            _audioCtx.decodeAudioData(item.ab).then(function (b) {
              _audioCache.set(item.key, b);
              _emitAudio(b, item.loop, item.noStop);
            }).catch(function (err) { console.warn('swinxAudioPlay: decode failed (pending)', err); });
          }
        })(q[i]);
      }
    }

    function _ensureAudioCtx() {
      if (_audioCtx) return _audioCtx;
      var AC = global.AudioContext || global.webkitAudioContext;
      if (!AC) return null;
      _audioCtx = new AC();
      _audioCtx.onstatechange = function () {
        if (_audioCtx.state === 'running') _flushPending();
      };
      return _audioCtx;
    }

    function _resumeAudio() {
      var ac = _ensureAudioCtx();
      if (ac && ac.state !== 'running') ac.resume().catch(function () {});
    }

    if (opt.audio) {
      global.addEventListener('pointerdown', _resumeAudio);
      global.addEventListener('keydown', _resumeAudio);
      global.addEventListener('pointerup', _resumeAudio);
      global.addEventListener('touchstart', _resumeAudio, { passive: true });
    }

    /* ---------------- 下载 .wasm / .data（带进度） ----------------
     * 加载器自己拉 .data 时没有进度回调，拉 .wasm 时连请求都不暴露，所以这里先带
     * 进度读进内存，再通过两个官方钩子交进去：
     *   Module.getPreloadedPackage —— .data，同步返回 ArrayBuffer，跳过它的下载；
     *   Module.instantiateWasm     —— .wasm，自己编译并回调，跳过它的 fetch。
     * fetch 带 cache:'no-cache'：产物名固定，不加的话浏览器可能复用旧缓存
     * （换构建后仍跑上一版，曾被误当成"控件缺陷"去查）。 */
    var FILES = [fWasm].concat(fData ? [fData] : []);
    var loadStat = { loaded: 0, total: 0 };
    var speedEma = 0, speedT = 0, speedBytes = 0, lastPaint = 0;

    function fmtMB(n) { return (n / 1048576).toFixed(1); }

    function paintLoad(force) {
      if (!loadEl) return;
      var now = performance.now();
      if (!force && now - lastPaint < 100) return; // 每个 chunk 都改 DOM 会拖慢下载
      lastPaint = now;
      if (loadEl.classList.contains('failed')) return;
      if (loadStat.total <= 0) {
        loadFillEl.style.width = '0%';
        loadMetaEl.textContent = speedEma > 0 ? speedEma.toFixed(1) + ' MB/s' : '0%';
        return;
      }
      var pct = Math.min(100, Math.floor((loadStat.loaded * 100) / loadStat.total));
      loadFillEl.style.width = pct + '%';
      var meta = pct + '% · ' + fmtMB(loadStat.loaded) + ' / ' + fmtMB(loadStat.total) + ' MB';
      if (speedEma > 0) meta += ' · ' + speedEma.toFixed(1) + ' MB/s';
      var rest = loadStat.total - loadStat.loaded;
      if (rest > 0 && speedEma > 0.2) {
        var eta = rest / (speedEma * 1048576);
        if (eta < 3600) meta += ' · 约 ' + (eta >= 10 ? Math.round(eta) : eta.toFixed(1)) + ' 秒';
      }
      loadMetaEl.textContent = meta;
    }

    function addBytes(n) {
      loadStat.loaded += n;
      var now = performance.now();
      if (!speedT) {
        speedT = now;
        speedBytes = loadStat.loaded;
      } else if (now - speedT >= 200) {
        var inst = (loadStat.loaded - speedBytes) / ((now - speedT) / 1000) / 1048576;
        speedEma = speedEma > 0 ? speedEma * 0.7 + inst * 0.3 : inst;
        speedT = now;
        speedBytes = loadStat.loaded;
      }
      paintLoad(false);
    }

    async function fetchFile(name) {
      var res = await fetch(name, { cache: 'no-cache' });
      if (!res.ok) throw new Error(name + ' 下载失败：HTTP ' + res.status);
      var total = Number(res.headers.get('Content-Length')) || 0;
      loadStat.total += total;
      // 无流式 body（老浏览器 / 某些代理）时退化成一次性读，进度只在结束时跳一次。
      if (!res.body || !res.body.getReader) {
        var buf = await res.arrayBuffer();
        addBytes(buf.byteLength);
        return buf;
      }
      var reader = res.body.getReader();
      var chunks = [];
      var received = 0;
      for (;;) {
        var r = await reader.read();
        if (r.done) break;
        chunks.push(r.value);
        received += r.value.length;
        addBytes(r.value.length);
      }
      var out = new Uint8Array(received);
      var off = 0;
      for (var i = 0; i < chunks.length; ++i) {
        out.set(chunks[i], off);
        off += chunks[i].length;
      }
      return out.buffer;
    }

    function hideLoading() {
      if (!loadEl || loadEl.classList.contains('done')) return;
      loadEl.classList.add('done');
      setTimeout(function () { loadEl.style.display = 'none'; }, 400);
    }

    /* ---------------- 胶水脚本载入 ----------------
     * 为什么不用静态 <script src="xxx.js">：.wasm/.data 是我们自己 fetch 的（每次都
     * 重新校验），胶水却是普通 script 标签 —— 静态服务器（python -m http.server
     * 之类）只发 Last-Modified 不发 Cache-Control 时，浏览器按启发式缓存直接复用
     * 旧文件 ⇒ 新 wasm 配旧胶水 ⇒
     *   LinkError: Import #7 "env" "_emval_decref": function import requires a callable
     * 加时间戳只是让它永远走网络（百来 KB，代价可忽略）。 */
    var glueLoaded = false, compiled = null;

    function loadGlue() {
      if (glueLoaded || typeof global[factoryName] === 'function') {
        glueLoaded = true;
        return Promise.resolve();
      }
      return new Promise(function (resolve, reject) {
        var s = document.createElement('script');
        s.src = fGlue + '?v=' + Date.now();
        s.onload = function () { glueLoaded = true; resolve(); };
        s.onerror = function () { reject(new Error('胶水脚本 ' + fGlue + ' 载入失败（404？）')); };
        document.head.appendChild(s);
      });
    }

    /* wasm 声明要哪些 import / 现有胶水提供了哪些。用于把 LinkError 翻译成
     * 「你拿到的是旧胶水」这种人话 —— 否则只能看到一句 requires a callable。 */
    function findMissingImports(mod, imports) {
      var miss = [];
      var list = WebAssembly.Module.imports(mod);
      for (var i = 0; i < list.length; ++i) {
        var it = list[i];
        if (it.kind !== 'function') continue;
        var ns = imports && imports[it.module];
        if (!ns || typeof ns[it.name] !== 'function') miss.push(it.module + '.' + it.name);
      }
      return miss;
    }

    /* ---------------- Module：交给胶水的实例配置 ---------------- */
    /* 上屏回调 / fatal 回调必须在 main() 跑起来之前就挂上：建窗期间那次同步绘制
     * 就会用到。onRuntimeInitialized 是导出函数生效之后、callMain 之前的时机。 */
    var Module = {
      canvas: canvas,
      print: function () {
        var a = Array.prototype.slice.call(arguments);
        console.log.apply(console, a);
        if (typeof opt.onLog === 'function') opt.onLog(a.join(' '));
      },
      printErr: function () { console.error.apply(console, Array.prototype.slice.call(arguments)); },
      swinxPresent: presentDirtyRect,
      souiFatal: function (why) { fail('启动失败: ' + why); },
      onRuntimeInitialized: onWasmReady,

      /* 剪贴板写出桥：wasm 里 SetClipboardData 后经 EM_ASM 调到这里。调用点在
       * Ctrl+C 的 keydown 处理器内同步发生（带用户激活，writeText 不会被拒）。
       * 失败静默：非安全上下文没有 navigator.clipboard，不能把复制链路弄崩。 */
      souixClipboardWrite: function (text) {
        try {
          if (navigator.clipboard && navigator.clipboard.writeText)
            navigator.clipboard.writeText(text).catch(function () {});
        } catch (err) {}
      },

      /* 音效桥：wasm 的 PlaySound 把 wav 字节 + 路径交到这里。
       * ptr=wav 字节指针（0 表示停止请求），len=字节数，
       * flags=fdwSound（SND_LOOP=0x8 / SND_NOSTOP=0x10），pathPtr=路径（缓存键）。
       * 只在 context 真正 running 时解码 + 播放，否则先 resume() 并排队 —— 否则
       * 带 SND_NOSTOP 的源会卡在 suspended 上 never-ended，后续音效被永久跳过。 */
      swinxAudioPlay: function (ptr, len, flags, pathPtr) {
        if (!opt.audio) return false;
        if (ptr === 0) {
          for (var i = 0; i < _audioSources.length; ++i) {
            try { _audioSources[i].stop(); } catch (e) {}
          }
          _audioSources = [];
          _audioPending = [];
          return true;
        }
        if (!len || len <= 0) return false;
        var ac = _ensureAudioCtx();
        if (!ac) return false;

        /* 拷出 wav 字节：wasm 侧在 EM_ASM 同步返回后立即 free，必须当场脱离 HEAPU8。
         * decodeAudioData 在部分 Chromium 上只接受裸 ArrayBuffer，传视图会抛
         * "parameter 1 is not of type 'ArrayBuffer'" 并被静默吞掉。 */
        var heap = Module.HEAPU8;
        var base = ptr >>> 0;
        var ab = new ArrayBuffer(len);
        new Uint8Array(ab).set(heap.subarray(base, base + len));

        var key = '';
        if (pathPtr) {
          var p = pathPtr >>> 0;
          while (heap[p] !== 0) { key += String.fromCharCode(heap[p]); p++; }
        }

        var loop = (flags & 0x0008) !== 0;    // SND_LOOP
        var noStop = (flags & 0x0010) !== 0;  // SND_NOSTOP

        if (_audioCache.has(key)) {
          if (ac.state === 'running') _emitAudio(_audioCache.get(key), loop, noStop);
          else {
            _audioPending.push({ cached: true, key: key, loop: loop, noStop: noStop });
            ac.resume().catch(function () {});
          }
          return true;
        }
        if (ac.state === 'running') {
          ac.decodeAudioData(ab).then(function (buf) {
            _audioCache.set(key, buf);
            _emitAudio(buf, loop, noStop);
          }).catch(function (err) { console.warn('swinxAudioPlay: decode failed', err); });
        } else {
          ac.resume().catch(function () {});
          _audioPending.push({ cached: false, ab: ab, key: key, loop: loop, noStop: noStop });
        }
        return true;
      },

      /* 文件选择桥：wasm 里 SwinxPickFileAsyncA 经 EM_ASM 调到这里（实现在 swinx 的
       * src/platform/wasm/dlghelper.cpp）。nId = swinx 的请求编号，title = 标题，
       * accept = swinx 从 lpstrFilter 换算出来的 <input accept>（空串 = 全部）。
       * 返回 false = 这次请求没发起，swinx 会 SetLastError 而不是把请求挂着。
       *
       * 三个浏览器事实决定了这里的形状：
       *   1. 文件选择框只认用户手势（navigator.userActivation.isActive 判瞬时激活；
       *      没有这个 API 的浏览器只能试，被拒时连事件都不发）；
       *   2. change 事件在本次 JS 栈返回之后才派发 —— 所以 swinx 那侧只能是异步契约；
       *   3. File 对象在事件回调之后就失效，所以要把整份字节读出来再交给
       *      _SwinxFilePickDone，由 swinx 落到 wasm 文件系统里换成一个可 fopen 的路径。
       * 取消：Chrome 87+/Safari 会发 cancel 事件；更老的浏览器不发，那条路径上 swinx
       * 的请求会一直占着槽位（8 个之后新请求显式失败，见 swinx 侧说明）。 */
      swinxPickFile: function (nId, title, accept) {
        var done = Module._SwinxFilePickDone;
        if (typeof done !== 'function' || !document.createElement) return false;
        if (navigator.userActivation && navigator.userActivation.isActive === false) {
          console.warn('swinxPickFile: 没有用户手势，浏览器不会打开选择框');
          return false;
        }
        var input = document.createElement('input');
        input.type = 'file';
        if (accept) input.accept = accept;
        input.style.display = 'none';
        var settled = false;
        /* 结果一律经这一个出口：把名字与字节写进线性内存、调重入口、再立刻 free
         * （与 souixClipboardWrite 同一所有权约定：C++ 只读，JS 负责释放）。 */
        function reply(nResult, name, buf) {
          if (settled) return;
          settled = true;
          if (input.parentNode) input.parentNode.removeChild(input);
          var pName = 0, pData = 0, nLen = 0;
          try {
            if (name) {
              var nb = new TextEncoder().encode(name);
              pName = Module._malloc(nb.length + 1);
              Module.HEAPU8.set(nb, pName);
              Module.HEAPU8[pName + nb.length] = 0;
            }
            if (buf && buf.byteLength > 0) {
              nLen = buf.byteLength;
              pData = Module._malloc(nLen);
              Module.HEAPU8.set(new Uint8Array(buf, 0, nLen), pData);
            }
            done(nId, nResult, pName, pData, nLen);
          } catch (err) {
            console.warn('swinxPickFile: 回投失败', err);
          } finally {
            if (pName) Module._free(pName);
            if (pData) Module._free(pData);
          }
        }
        input.onchange = function () {
          var f = input.files && input.files[0];
          if (!f) { reply(0, null, null); return; }
          var p = f.arrayBuffer ? f.arrayBuffer() : new Promise(function (res, rej) {
            var fr = new FileReader();
            fr.onload = function () { res(fr.result); };
            fr.onerror = function () { rej(fr.error); };
            fr.readAsArrayBuffer(f);
          });
          p.then(function (ab) { reply(1, f.name, ab); },
                 function (err) { console.warn('swinxPickFile: 读取失败', err); reply(-1, null, null); });
        };
        input.oncancel = function () { reply(0, null, null); };
        /* Firefox 要求元素在文档里才允许 click()，Chrome 不要求 —— 挂上再摘掉，两边都能用。 */
        if (document.body) document.body.appendChild(input);
        input.click();
        return true;
      }
    };

    /* ---------------- 上屏 ---------------- */
    function ensureSurface() {
      /* DPI 档位在 wasm main() 开头才设置（SwinxSetCanvasScale），而
       * onRuntimeInitialized 早于 main()：fb 若按默认逻辑尺寸建过，第一帧上屏
       * 时检测到 C++ 侧尺寸变了就整体重建。 */
      var w = Module._SwinxFbWidth();
      var h = Module._SwinxFbHeight();
      if (fb && (fb.w !== w || fb.h !== h)) { fb = null; img = null; img32 = null; }
      if (fb) return true;
      if (!ready || !Module.HEAPU32) return false;
      var stride = Module._SwinxFbStride();
      if (w < 1 || h < 1 || stride < w) {
        fail('帧缓冲尺寸异常: ' + w + 'x' + h + ' stride ' + stride);
        return false;
      }
      canvas.width = w;
      canvas.height = h;
      /* 显示尺寸交给 CSS（max-width:100% + height:auto）。不写 canvas.style.width
       * 是因为要留 max-width 收窄的余地；逻辑宽度用 CSS 变量给（见上面的样式）。
       * 物理宽 = 逻辑宽 x DPI 档位，所以这里必须用逻辑尺寸，不能用 w。 */
      var lw = opt.width, lh = opt.height;
      if (!lw && typeof Module._SwinxFbLogicalWidth === 'function') lw = Module._SwinxFbLogicalWidth();
      if (!lh && typeof Module._SwinxFbLogicalHeight === 'function') lh = Module._SwinxFbLogicalHeight();
      if (lw) canvas.style.setProperty('--soui-logical-w', lw + 'px');
      if (lh) canvas.style.setProperty('--soui-logical-h', lh + 'px');
      img = ctx.createImageData(w, h);
      img32 = new Uint32Array(img.data.buffer);
      fb = { w: w, h: h, stride: stride };
      return true;
    }

    var PERF = location.search.indexOf('perf=1') >= 0;
    var perfSum = 0, perfCnt = 0, perfPix = 0;
    global.__perfFrames = 0;
    global.__perfLast = 0;

    function perfAccum(t0, px) {
      perfSum += performance.now() - t0;
      perfPix += px;
      ++global.__perfFrames;
      global.__perfLast = performance.now() - t0;
      if (++perfCnt >= 10) {
        console.log('[present] avg=' + (perfSum / perfCnt).toFixed(2) + 'ms avgPx=' +
          Math.round(perfPix / perfCnt) + ' fb=' + fb.w + 'x' + fb.h +
          (Module.fbFormat === 'rgba' ? ' rgba' : Module.fbFormat === 'canvas' ? ' canvas' : ' argb'));
        perfSum = 0;
        perfCnt = 0;
        perfPix = 0;
      }
    }

    function presentDirtyRect(l, t, r, b) {
      if (!ready || !ensureSurface()) return;
      if (l < 0) l = 0;
      if (t < 0) t = 0;
      if (r > fb.w) r = fb.w;
      if (b > fb.h) b = fb.h;
      if (l >= r || t >= b) return;

      // ?perf=1：统计上屏耗时，判断"卡在回读像素"还是"卡在绘制调用"。
      var t0 = PERF ? performance.now() : 0;
      var w = r - l;
      var h = b - t;

      // ---- canvas2d 后端：像素已经由 C++ 侧 drawImage 合成到显示画布了 ----
      if (Module.fbFormat === 'canvas') {
        if (PERF) perfAccum(t0, w * h);
        return;
      }

      // ---- 快路径：fb 已是浏览器原生 RGBA 字节序（canvas2d 后端）----
      if (Module.fbFormat === 'rgba') {
        var bytes = Module.HEAPU8;   // 扩容后视图会变，每次重取
        var fbPtr = Module._SwinxFbBits() + (t * fb.stride + l) * 4;
        if (l === 0 && w === fb.w) {
          // 脏区覆盖整行宽：内存里连续，一次构造
          var view = new Uint8ClampedArray(bytes.buffer, fbPtr, w * h * 4);
          ctx.putImageData(new ImageData(view, w, h), l, t);
        } else {
          /* 窄脏区：先按行 memcpy 进常驻的整屏 ImageData，再一次 putImageData。
           * 绝不能"每行一次 putImageData + 每行 new 一个 ImageData"：局部脏矩形时
           * 一帧几百次上屏，切页时能从 35 次暴涨到 1 万次。 */
          var dst = img.data;
          var rowBytes = w * 4;
          for (var y = 0; y < h; ++y) {
            var off = fbPtr + y * fb.stride * 4;
            /* 目标行号必须是绝对行 t+y：putImageData 的 (l,t,w,h) 是按 img 自身
             * 坐标系给的，写相对行号会让内容整体位移 t 像素。 */
            dst.set(new Uint8ClampedArray(bytes.buffer, off, rowBytes), ((t + y) * fb.w + l) * 4);
          }
          ctx.putImageData(img, 0, 0, l, t, w, h);
        }
        if (PERF) perfAccum(t0, w * h);
        return;
      }

      // ---- 兜底：fb 是 cairo ARGB32（预乘，小端 = 0xAARRGGBB），逐像素换序 ----
      var heap = Module.HEAPU32;   // 扩容后要重新取视图
      var base = Module._SwinxFbBits() >>> 2;
      for (var yy = t; yy < b; ++yy) {
        var src = base + yy * fb.stride + l;
        var d = yy * fb.w + l;
        for (var x = 0; x < w; ++x) {
          var v = heap[src + x];
          img32[d + x] = (v & 0xFF00FF00) | ((v & 0xFF) << 16) | ((v >> 16) & 0xFF);
        }
      }
      ctx.putImageData(img, 0, 0, l, t, w, h);
      if (PERF) perfAccum(t0, w * h);
    }

    /* ---------------- Win32 常量（与 swinx/include 一致，JS 侧只能硬编码） --- */
    var WM_MOUSEMOVE = 0x0200, WM_LBUTTONDOWN = 0x0201, WM_LBUTTONUP = 0x0202;
    var WM_RBUTTONDOWN = 0x0204, WM_RBUTTONUP = 0x0205;
    var WM_MBUTTONDOWN = 0x0207, WM_MBUTTONUP = 0x0208;
    var WM_MOUSEWHEEL = 0x020A, WHEEL_DELTA = 120;
    var WM_KEYDOWN = 0x0100, WM_KEYUP = 0x0101;
    var WM_SYSKEYDOWN = 0x0104, WM_SYSKEYUP = 0x0105;
    var MK_LBUTTON = 0x0001, MK_RBUTTON = 0x0002, MK_MBUTTON = 0x0010;
    var VK_BACK = 0x08, VK_TAB = 0x09, VK_RETURN = 0x0D, VK_SHIFT = 0x10,
      VK_CONTROL = 0x11, VK_MENU = 0x12, VK_PAUSE = 0x13, VK_CAPITAL = 0x14,
      VK_ESCAPE = 0x1B, VK_SPACE = 0x20, VK_PRIOR = 0x21, VK_NEXT = 0x22,
      VK_END = 0x23, VK_HOME = 0x24, VK_LEFT = 0x25, VK_UP = 0x26,
      VK_RIGHT = 0x27, VK_DOWN = 0x28, VK_INSERT = 0x2D, VK_DELETE = 0x2E;
    /* OEM 标点 / 小键盘 / Win 键。这些是**键位**不是字符：字符由 swinx 的
     * TranslateMessage 按 Shift/CapsLock 状态算，DOM 层只报物理键。 */
    var VK_LWIN = 0x5B, VK_NUMPAD0 = 0x60, VK_MULTIPLY = 0x6A, VK_ADD = 0x6B,
      VK_SUBTRACT = 0x6D, VK_DECIMAL = 0x6E, VK_DIVIDE = 0x6F, VK_F1 = 0x70,
      VK_NUMLOCK = 0x90, VK_SCROLL = 0x91,
      VK_OEM_1 = 0xBA, VK_OEM_PLUS = 0xBB, VK_OEM_COMMA = 0xBC,
      VK_OEM_MINUS = 0xBD, VK_OEM_PERIOD = 0xBE, VK_OEM_2 = 0xBF,
      VK_OEM_3 = 0xC0, VK_OEM_4 = 0xDB, VK_OEM_5 = 0xDC, VK_OEM_6 = 0xDD,
      VK_OEM_7 = 0xDE;

    var KEY_MAP = {
      Backspace: VK_BACK, Tab: VK_TAB, Enter: VK_RETURN, Shift: VK_SHIFT,
      Control: VK_CONTROL, Alt: VK_MENU, Pause: VK_PAUSE, CapsLock: VK_CAPITAL,
      Escape: VK_ESCAPE, ' ': VK_SPACE, PageUp: VK_PRIOR, PageDown: VK_NEXT,
      End: VK_END, Home: VK_HOME, ArrowLeft: VK_LEFT, ArrowUp: VK_UP,
      ArrowRight: VK_RIGHT, ArrowDown: VK_DOWN, Insert: VK_INSERT, Delete: VK_DELETE
    };
    var CODE_MAP = {
      Space: VK_SPACE, Backquote: VK_OEM_3, Minus: VK_OEM_MINUS, Equal: VK_OEM_PLUS,
      BracketLeft: VK_OEM_4, BracketRight: VK_OEM_6, Backslash: VK_OEM_5,
      Semicolon: VK_OEM_1, Quote: VK_OEM_7, Comma: VK_OEM_COMMA,
      Period: VK_OEM_PERIOD, Slash: VK_OEM_2,
      ShiftLeft: VK_SHIFT, ShiftRight: VK_SHIFT, ControlLeft: VK_CONTROL,
      ControlRight: VK_CONTROL, AltLeft: VK_MENU, AltRight: VK_MENU,
      MetaLeft: VK_LWIN, MetaRight: VK_LWIN,
      NumLock: VK_NUMLOCK, ScrollLock: VK_SCROLL, NumpadEnter: VK_RETURN,
      NumpadMultiply: VK_MULTIPLY, NumpadAdd: VK_ADD, NumpadSubtract: VK_SUBTRACT,
      NumpadDecimal: VK_DECIMAL, NumpadDivide: VK_DIVIDE
    };

    /* VK 必须按**物理键位**（e.code）算，不能按 e.key：Shift+1 的 e.key 是 '!'
     * （0x21，撞上 VK_PRIOR），'[' 是 0x5B（VK_LWIN），'.' 是 0x2E（VK_DELETE）。
     * e.key 只在没有 e.code 的老浏览器 / IME 合成事件里兜底。 */
    function vkFromEvent(e) {
      var code = e.code || '';
      var key = e.key;
      if (/^Key[A-Z]$/.test(code)) return code.charCodeAt(3);
      if (/^Digit[0-9]$/.test(code)) return code.charCodeAt(5);
      var m = /^Numpad([0-9])$/.exec(code);
      if (m) return VK_NUMPAD0 + Number(m[1]);
      m = /^F([1-9]|1[0-2])$/.exec(code);
      if (m) return VK_F1 + Number(m[1]) - 1;
      if (CODE_MAP[code] !== undefined) return CODE_MAP[code];
      if (KEY_MAP[code] !== undefined) return KEY_MAP[code];
      if (key === undefined || key === 'Unidentified') return 0;
      if (KEY_MAP[key] !== undefined) return KEY_MAP[key];
      if (key.length === 1) return key.toUpperCase().charCodeAt(0);
      return 0;
    }

    function canvasPoint(e) {
      var rc = canvas.getBoundingClientRect();
      // DPI 缩放由 SOUI 在布局层完成：应用坐标 == 帧缓冲物理坐标，
      // 鼠标坐标按 canvas 显示尺寸与物理分辨率的比值映射即可。
      var x = Math.floor(((e.clientX - rc.left) * fb.w) / rc.width);
      var y = Math.floor(((e.clientY - rc.top) * fb.h) / rc.height);
      return [Math.max(0, Math.min(fb.w - 1, x)), Math.max(0, Math.min(fb.h - 1, y))];
    }

    function buttonMessages(e) {
      var keys = 0;
      if (e.buttons & 1) keys |= MK_LBUTTON;
      if (e.buttons & 2) keys |= MK_RBUTTON;
      if (e.buttons & 4) keys |= MK_MBUTTON;
      return keys;
    }

    /* C++ 侧可能没导出某个口（最小程序可以不要 IME）。缺了就当没有，不要抛。 */
    function hasFn(name) { return typeof Module[name] === 'function'; }
    function callFn(name, a, b, c, d) {
      var f = Module[name];
      if (typeof f !== 'function') return undefined;
      return f(a, b, c, d);
    }

    /* ---------------- 输入法：代理输入框 ---------------- */
    function focusProxy() {
      if (!proxy) return;
      try { proxy.focus({ preventScroll: true }); } catch (err) { proxy.focus(); }
    }
    function hasKeyFocus() {
      var a = document.activeElement;
      return a === proxy || a === canvas;
    }

    var caretBuf = 0;
    /* 把代理框钉在 SOUI 的插入符上：浏览器的候选词窗跟着焦点元素走，代理在光标处，
     * 候选窗就在正在输入的那行字旁边。每帧一次，成本是读两个 int。 */
    function syncProxyPos() {
      if (!proxy || !ready || !fb || !hasFn(api.caret)) return;
      if (!caretBuf) caretBuf = Module._malloc(8);
      var heap = Module.HEAP32;   // ALLOW_MEMORY_GROWTH 扩容后要重新取视图
      if (!Module[api.caret](caretBuf)) return;
      var rc = canvas.getBoundingClientRect();
      var x = heap[caretBuf >> 2], y = heap[(caretBuf >> 2) + 1];
      proxy.style.left = Math.round(rc.left + (x * rc.width) / fb.w) + 'px';
      proxy.style.top = Math.round(rc.top + (y * rc.height) / fb.h - 2) + 'px';
    }

    /* 一次输入法提交：UTF-8 送进 wasm，由 SwinxInjectText 拆成逐码点的 WM_IME_CHAR。 */
    function commitImeText(s) {
      if (!ready || !s) return;
      /* 终止符写进堆，不写在源码里。-Os/Release 下 emscripten 会压缩 --shell-file
       * 产出的 html，把 JS 字面量 '\0' 落成裸 0x00 字节；浏览器按 HTML 规范把
       * script 数据里的 NUL 换成 U+FFFD，于是每次提交都多一个乱码字符。数字 0
       * 没有这种写法。（本文件是独立的 .js，不经过 html 压缩，但保持一致写法。） */
      var bytes = new TextEncoder().encode(s);
      var p = Module._malloc(bytes.length + 1);
      try {
        Module.HEAPU8.set(bytes, p);
        Module.HEAPU8[p + bytes.length] = 0;
        callFn(api.text, p);
        callFn(api.pump);   // 和注入按键同一个理由：立刻抽干，别等下一帧
      } finally {
        Module._free(p);
      }
      syncProxyPos();
    }

    function hookImeProxy() {
      if (!proxy) return;
      proxy.addEventListener('focus', function () { canvas.classList.add('keyed'); });
      proxy.addEventListener('blur', function () {
        canvas.classList.remove('keyed');
        composing = false;
      });
      /* 上一段组合结束后**必须**复位 composing：否则之后在输入法英文模式下打
       * ASCII 时 composing 仍为真，input 处理器会把已提交的 insertText 一起跳过，
       * 表现为"中文能打、英文/符号无响应"。 */
      var lastComposeData = '';
      var lastComposeAt = 0;
      proxy.addEventListener('compositionstart', function () { composing = true; });
      proxy.addEventListener('compositionend', function (e) {
        composing = false;
        lastComposeData = e.data || '';
        lastComposeAt = performance.now();
        commitImeText(lastComposeData);
        proxy.value = '';
      });
      /* 手机软键盘、表情面板、"直接上屏"、输入法英文模式打 ASCII 都不产生组合
       * 事件，只发 input(insertText)。区分预编辑与已提交唯一可靠的信号是
       * e.inputType：insertCompositionText 是拼音串（绝不能提交），insertText 是
       * 成品（必须提交）。用 `composing || e.isComposing` 整体跳过会把英文模式的
       * ASCII 一并挡掉。 */
      proxy.addEventListener('input', function (e) {
        if (e.inputType === 'insertCompositionText') return;
        if (e.inputType === 'insertText' && e.data) {
          if (!composing && lastComposeData && performance.now() - lastComposeAt < 40 &&
            e.data === lastComposeData) return;   // 组合刚结束时的重复 insertText
          commitImeText(e.data);
        }
        proxy.value = '';
      });
    }

    /* ---------------- 鼠标 / 键盘 / 滚轮 / 剪贴板 ---------------- */
    function hookInput() {
      canvas.addEventListener('contextmenu', function (e) { e.preventDefault(); });

      canvas.addEventListener('pointermove', function (e) {
        if (!ready || !fb) return;
        var p = canvasPoint(e);
        callFn(api.mouse, WM_MOUSEMOVE, p[0], p[1], buttonMessages(e));
      });

      var downMsg = { 0: WM_LBUTTONDOWN, 1: WM_MBUTTONDOWN, 2: WM_RBUTTONDOWN };
      var upMsg = { 0: WM_LBUTTONUP, 1: WM_MBUTTONUP, 2: WM_RBUTTONUP };

      canvas.addEventListener('pointerdown', function (e) {
        if (!ready || !fb) return;
        e.preventDefault();
        focusProxy();
        var p = canvasPoint(e);
        callFn(api.mouse, downMsg[e.button] || WM_LBUTTONDOWN, p[0], p[1], buttonMessages(e));
      });

      /* pointerup 挂 window 不挂 canvas：鼠标移出画布后释放时 pointerup 落在光标
       * 下的其它元素上（鼠标没有隐式捕获），挂 canvas 上就永远收不到 —— 按键粘在
       * 按下态，richedit 弹起后仍会继续扩展选区。window 级同时兜住 pointercancel。 */
      global.addEventListener('pointerup', function (e) {
        if (!ready || !fb) return;
        var p = canvasPoint(e);
        callFn(api.mouse, upMsg[e.button] || WM_LBUTTONUP, p[0], p[1], buttonMessages(e));
      });
      global.addEventListener('pointercancel', function (e) {
        if (!ready || !fb) return;
        var p = canvasPoint(e);
        callFn(api.mouse, WM_LBUTTONUP, p[0], p[1], 0);
      });
      // 页面失焦（切标签页/切窗口）时物理按键必然全部释放
      global.addEventListener('blur', function () {
        if (!ready || !fb) return;
        callFn(api.mouse, WM_LBUTTONUP, -1, -1, 0);
        callFn(api.mouse, WM_RBUTTONUP, -1, -1, 0);
        callFn(api.mouse, WM_MBUTTONUP, -1, -1, 0);
      });

      /* 滚轮：先归 SOUI，SOUI 明确返回"没消费"后才把这一下交给页面。
       * 不能靠"不 preventDefault"把滚轮让给浏览器：默认滚动在事件派发之后才发生，
       * 浏览器会先滚页面、页面到底后才轮到 SOUI，顺序正好反了。所以这里始终
       * preventDefault，SOUI 没消费时再用 scrollBy 手动补上页面滚动量。
       * Win32 用 ±WHEEL_DELTA（120）的整数倍表达一格，浏览器给的是像素/行/页三种
       * 单位：先按"格"记小数，攒够一整格才注入（否则触控板几像素的增量会被取整成 0）。
       * 注入值必须还原成 120 的整数倍——SPanel::OnMouseWheel 自己也按 120 攒格，
       * 传 4 这种"格数"会被它当成不足一格，返回"没消费"，页面跟着滚。
       * 不足一格时不能立刻交给页面：触控板还在列表上，页面先滚几像素、又把攒下的
       * 小数清零，列表就永远攒不满一格。只有"指针没动且上一次整格明确归页面"时
       * 才把小数增量直接给页面，其余情况先攒着等整格判定。
       * Ctrl/⌘+滚轮是浏览器缩放，不拦也不注入。 */
      var wheelRest = 0;
      /* 上一次整格注入的指针位置（viewport 坐标：页面滚动会把画布移走，帧缓冲
       * 坐标不能拿来做"指针有没有动"的比较）和消费结果。 */
      var wheelAnchor = null;
      var WHEEL_ANCHOR_SLOP = 20;
      function wheelPageScroll(e) {
        if (e.deltaMode === 1) return e.deltaY * 16; /* 行 */
        if (e.deltaMode === 2) return e.deltaY * (window.innerHeight || 800); /* 页 */
        return e.deltaY; /* 像素 */
      }
      /* 一整格对应的页面滚动距离：整格被 SOUI 拒绝时按这个补，不能用当时那个
       * 原始事件的小增量（整格是攒出来的）。 */
      function wheelNotchScroll(e) {
        if (e.deltaMode === 1) return 3 * 16;
        if (e.deltaMode === 2) return (window.innerHeight || 800);
        return 100;
      }
      canvas.addEventListener('wheel', function (e) {
        if (!ready || !fb) return;
        if (e.ctrlKey || e.metaKey) return;
        e.preventDefault();
        var p = canvasPoint(e);
        var keys = buttonMessages(e);
        var perNotch = e.deltaMode === 1 ? 3 : (e.deltaMode === 2 ? 1 : 100);
        wheelRest += (-e.deltaY) / perNotch;
        var whole = wheelRest < 0 ? -Math.floor(-wheelRest) : Math.floor(wheelRest);
        if (whole) {
          wheelRest -= whole;
          /* C++ 侧 SwinxDemoWheel 会先补一条同坐标的 WM_MOUSEMOVE（SHostWnd 用
           * 最近一条移动消息做命中测试），再同步派发滚轮，所以这里不单独注入。
           * zDelta 按 Win32 语义给 ±120 的整数倍，SOUI 才能按整格判定消费。 */
          var ret = callFn(api.wheel, p[0], p[1], whole * WHEEL_DELTA, keys);
          wheelAnchor = { x: e.clientX, y: e.clientY, handled: !!ret };
          if (!ret) {
            window.scrollBy(0, -whole * wheelNotchScroll(e));
            wheelRest = 0;
          }
        } else {
          /* 指针没动、上一次整格明确归页面：小数增量直接给页面，触控板才平滑；
           * 指针换了位置（可能已挪到可滚控件上）就先攒着，等整格判定。 */
          var near = wheelAnchor &&
                     Math.abs(e.clientX - wheelAnchor.x) <= WHEEL_ANCHOR_SLOP &&
                     Math.abs(e.clientY - wheelAnchor.y) <= WHEEL_ANCHOR_SLOP;
          if (near && !wheelAnchor.handled) {
            window.scrollBy(0, wheelPageScroll(e));
            wheelRest = 0;
          }
        }
      }, { passive: false });

      /* 键盘：只有画面拿到焦点后才注入，避免吃掉页面本身的快捷键；Tab 留给页面导航。
       * 每次注入后立刻抽干队列（不只是优化，是正确性要求）：wasm 的
       * TranslateMessage 在"翻译时刻"读 GetKeyState(VK_SHIFT/VK_CAPITAL)，而真实
       * Win32 用的是按键进队时刻的快照。一次回调里连投 down/up 时，Shift 的 keyup
       * 会在任何一次泵之前就把状态位清掉，于是 Shift+A 翻出小写 a。 */
      var sysKey = function (e) { return e.altKey || e.key === 'F10'; };
      var drainKeys = function () { try { callFn(api.pump); } catch (err) {} };
      var isComposing = function (e) {
        return composing || e.isComposing === true || e.keyCode === 229;
      };

      global.addEventListener('keydown', function (e) {
        if (!ready) return;
        if (e.target && e.target !== proxy && /INPUT|TEXTAREA|SELECT|BUTTON/.test(e.target.tagName)) return;
        if (!hasKeyFocus()) return;
        if (isComposing(e)) return;
        var vk = vkFromEvent(e);
        if (!vk) return;
        /* Ctrl+V 不拦截不注入：keydown 上 preventDefault 会把随后的 paste 事件一并
         * 取消，而剪贴板文本只能从 paste 事件的 clipboardData 同步拿到。 */
        if (vk === 0x56 && e.ctrlKey && !e.altKey && !e.metaKey) return;
        if (vk !== VK_TAB) e.preventDefault();
        callFn(api.key, sysKey(e) ? WM_SYSKEYDOWN : WM_KEYDOWN, vk, 0);
        drainKeys();
      });

      global.addEventListener('keyup', function (e) {
        if (!ready) return;
        if (!hasKeyFocus()) return;
        if (isComposing(e)) return;
        var vk = vkFromEvent(e);
        if (!vk) return;
        callFn(api.key, sysKey(e) ? WM_SYSKEYUP : WM_KEYUP, vk, 0);
        drainKeys();
      });

      /* 剪贴板读入：paste 事件带同步可读的 clipboardData。
       *   1. 文本经 SwinxClipboardSetBrowserText 写进 swinx 的 SClipboard；
       *   2. 注入 V 的 down/up（Ctrl 的 down 已由 keydown 注入），richedit 收到
       *      WM_KEYDOWN 'V' 走标准 WM_PASTE -> GetClipboardData；
       *   3. 同步抽干队列，让粘贴在本次事件回调内完成。 */
      global.addEventListener('paste', function (e) {
        if (!ready || !fb) return;
        if (!hasKeyFocus()) return;
        if (composing) return;
        var text = (e.clipboardData && e.clipboardData.getData) ? e.clipboardData.getData('text') : '';
        e.preventDefault();
        if (text && hasFn(api.clipboard)) {
          var bytes = new TextEncoder().encode(text);
          var p = Module._malloc(bytes.length + 1);
          try {
            Module.HEAPU8.set(bytes, p);
            Module.HEAPU8[p + bytes.length] = 0;
            Module[api.clipboard](p);
          } finally {
            Module._free(p);
          }
        }
        callFn(api.key, WM_KEYDOWN, 0x56, 0);
        callFn(api.key, WM_KEYUP, 0x56, 0);
        drainKeys();
      });
    }

    /* ---------------- 每帧任务 ---------------- */
    /* 唯一要每帧做的是把 IME 代理框钉到 SOUI 的插入符上。其余时间这个循环只是
     * 空转等输入 —— 画面本身由 C++ 的 emscripten_set_main_loop 驱动。 */
    function tick() {
      if (ready && fb) syncProxyPos();
      requestAnimationFrame(tick);
    }

    /* ---------------- 启动 ---------------- */
    function onWasmReady() {
      ready = true;
      if (!ensureSurface()) {
        fail('运行时已就绪但拿不到帧缓冲尺寸');
        return;
      }
      presentDirtyRect(0, 0, fb.w, fb.h);
      hideLoading();
      if (resolveReady) { resolveReady(hook); resolveReady = null; }
    }

    async function startLoad() {
      if (loadPhaseEl) loadPhaseEl.textContent = '正在下载模块与资源…';
      paintLoad(true);
      try {
        var buffers = await Promise.all(FILES.map(fetchFile));
        var wasmBuf = buffers[0];
        var dataBuf = buffers.length > 1 ? buffers[1] : null;
        speedEma = 0;
        if (loadFillEl) loadFillEl.style.width = '100%';
        if (loadMetaEl) loadMetaEl.textContent = fmtMB(loadStat.loaded) + ' MB 下载完成';
        if (loadPhaseEl) loadPhaseEl.textContent = '正在编译并初始化…';
        await loadGlue();
        /* 先自己编译一遍：既省掉 instantiate(bytes) 的二次编译，也能在实例化之前
         * 拿到「wasm 要哪些 import」，把版本对不上翻译成人话。 */
        compiled = await WebAssembly.compile(wasmBuf);
        if (dataBuf) Module.getPreloadedPackage = function () { return dataBuf; };
        Module.instantiateWasm = function (imports, successCallback) {
          var miss = findMissingImports(compiled, imports);
          if (miss.length) {
            fail('JS 胶水与 wasm 不是同一次构建（缺少 ' + miss.slice(0, 3).join('、') +
              (miss.length > 3 ? ' 等 ' + miss.length + ' 项' : '') +
              '）。刷新一下就好：Ctrl+F5（或在开发者工具 Network 里勾 Disable cache 再重载）。');
            return {};
          }
          WebAssembly.instantiate(compiled, imports).then(
            function (inst) { successCallback(inst, compiled); },
            function (e) { fail('WebAssembly 实例化失败：' + e); });
          return {};
        };
        var factory = global[factoryName];
        if (typeof factory !== 'function') {
          throw new Error('胶水脚本没有定义工厂函数 ' + factoryName + '（emcc -sEXPORT_NAME）');
        }
        await factory(Module);
        if (!ready) onWasmReady();
      } catch (e) {
        fail('加载失败：' + (e && e.message ? e.message : e));
      }
    }

    hookImeProxy();
    hookInput();
    requestAnimationFrame(tick);

    /* 崩溃上报：wasm OOB 之类 trap 只有 error.stack 里有线索（链接期 -g 保留 name
     * 段时，栈里直接是 C++ 函数名）。onerror 第 5 参和 onunhandledrejection 的
     * reason 才带 stack，两者分开接、完整打印。 */
    global.onerror = function (ev, src, line, col, err) {
      var msg = (err && err.message) || (ev && ev.message) || String(ev);
      fail('运行时异常，详见控制台: ' + msg);
      if (err && err.stack) console.error('[soui] crash stack:\n' + err.stack);
    };
    global.onunhandledrejection = function (ev) {
      var r = ev && ev.reason;
      var msg = (r && (r.message || r.toString())) || String(ev);
      fail('运行时异常，详见控制台: ' + msg);
      if (r && r.stack) console.error('[soui] crash stack:\n' + r.stack);
      else console.error('[soui] unhandled rejection:', r);
    };

    /* 挂到 window 上：页面控制台、诊断脚本（tools/diag/*）和 e2e 都用 Module.xxx
     * 读帧缓冲状态。MODULARIZE 下胶水不依赖全局 Module（它用传进去的那个对象），
     * 所以这里赋值不会和加载器打架。 */
    global.Module = Module;

    var hook = {
      canvas: canvas,
      module: Module,
      options: opt,
      fail: fail,
      whenReady: whenReady,
      /* 程序跑起来之后想再喂输入（自测/自动化）时用：坐标是逻辑坐标。 */
      injectText: commitImeText,
      requestFocus: focusProxy
    };

    global.SouiHost.current = hook;
    startLoad();
    return hook;
  }

  global.SouiHost = { start: start, version: VERSION };
})(window);
