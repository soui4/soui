/*
 * Index.ets
 *
 * 纯 ArkTS 鸿蒙刷新率测试页（不依赖 SOUI / N-API）。
 *
 * 目的：对比 ArkUI 各种"CPU 位图 → 上屏"载体的性能，定位 SOUI for OHOS
 *       全屏刷新慢（putImageData ~40ms）的瓶颈，并找出更快的上屏方式。
 *
 * 背景：putImageData 走软件路径（CPU 写后备存储 + 上传），全屏 10MB 缓冲区
 *       在模拟器上单帧 >40ms。改用 PixelMap 作为 GPU 友好载体，drawImage 走
 *       GPU 纹理路径；或用 RenderNode + ArkGraphics2D drawing.Canvas 直接在
 *       GPU 渲染节点上绘制。
 *
 * 载体（carrier）四档，点按钮循环切换：
 *   0 putImageData         —— CanvasRenderingContext2D.putImageData（软件基线）
 *   1 drawImage(PixelMap)  —— drawImage(PixelMap) 走 GPU 纹理
 *   2 RenderNode           —— RenderNode + drawing.Canvas GPU 渲染节点
 *   3 原生绘制             —— createLinearGradient+fillRect（GPU 天花板基线）
 *
 * 刷新范围：全屏 / 局部（仅对 putImageData 有意义，测小块 dirty 上传）
 *
 * 解读：
 *   - drawImage(PixelMap) FPS >> putImageData FPS → putImageData 软件路径是瓶颈，
 *     SOUI 应改用 PixelMap+drawImage 上屏（C++ 出 RGBA → createPixelMapSync/
 *     writePixelsSync → drawImage）。
 *   - RenderNode FPS 最高 → 走 ArkGraphics2D 渲染节点是终极方案。
 *   - 原生绘制 FPS = GPU 天花板；都达不到说明别处有瓶颈。
 */

import { LengthMetricsUnit, NodeController, FrameNode, RenderNode, DrawContext, UIContext } from '@kit.ArkUI';
import { image } from '@kit.ImageKit';
import { hilog } from '@kit.PerformanceAnalysisKit';

const DOMAIN = 0x5350;
const TAG = 'RefreshTest';

// 载体编号
const CARRIER_PUTIMAGE: number = 0;
const CARRIER_DRAWPIXELMAP: number = 1;
const CARRIER_RENDERNODE: number = 2;
const CARRIER_NATIVE: number = 3;
const CARRIER_NAMES: string[] = ['putImageData', 'drawImage(PixelMap)', 'RenderNode(GPU)', '原生绘制'];

/* ===================== RenderNode + drawing.Canvas GPU 路径 ===================== */

class RefreshRenderNode extends RenderNode {
  pixelMap: image.PixelMap | null = null;
  async draw(context: DrawContext): Promise<void> {
    if (this.pixelMap) {
      // drawing.Canvas.drawImage(PixelMap)，GPU 纹理绘制
      context.canvas.drawImage(this.pixelMap, 0, 0);
    }
  }
}

class RefreshNodeController extends NodeController {
  private rootNode: FrameNode | null = null;
  private renderNode: RefreshRenderNode = new RefreshRenderNode();
  makeNode(uiContext: UIContext): FrameNode {
    this.rootNode = new FrameNode(uiContext);
    const rn = this.rootNode.getRenderNode();
    if (rn !== null) {
      this.renderNode.frame = { x: 0, y: 0, width: 10000, height: 10000 };
      rn.appendChild(this.renderNode);
    }
    return this.rootNode;
  }
  setFrame(w: number, h: number): void {
    this.renderNode.frame = { x: 0, y: 0, width: w, height: h };
  }
  setPixelMap(pm: image.PixelMap | null): void {
    this.renderNode.pixelMap = pm;
    this.renderNode.invalidate();
  }
}

/* ===================== 测试页 ===================== */

@Entry
@Component
struct Index {
  /** Canvas 上下文，PX 单位模式（与 SouiSurface 一致） */
  private ctx: CanvasRenderingContext2D =
    new CanvasRenderingContext2D(new RenderingContextSettings(true), LengthMetricsUnit.PX);
  /** 捕获的渐变 ImageData A/B（用于 putImageData 载体） */
  private imageDataA: ImageData | null = null;
  private imageDataB: ImageData | null = null;
  /** 从 ImageData 缓冲区构建的 PixelMap A/B（用于 drawImage / RenderNode 载体） */
  private pixelMapA: image.PixelMap | null = null;
  private pixelMapB: image.PixelMap | null = null;
  /** 缓冲区物理像素宽高 */
  private pw: number = 0;
  private ph: number = 0;
  /** Canvas 是否就绪 */
  private canvasReady: boolean = false;
  /** 渲染循环是否在跑 */
  private looping: boolean = false;
  private frameTs: number[] = [];
  private frameCounter: number = 0;
  private lastHudTs: number = 0;

  /** 局部刷新小块参数 */
  private dirtyX: number = 0;
  private dirtyY: number = 0;
  private dirtyDirX: number = 6;
  private dirtyDirY: number = 4;
  private readonly kDirtyW: number = 240;
  private readonly kDirtyH: number = 240;

  /** RenderNode 控制器 */
  private nodeController: RefreshNodeController = new RefreshNodeController();

  // ===== 测试开关 =====
  @State private carrier: number = CARRIER_PUTIMAGE;
  @State private modeDirty: boolean = false;
  @State private running: boolean = true;

  // ===== HUD =====
  @State private fps: number = 0;
  @State private frameMs: number = 0;
  @State private hud: string = '';

  build() {
    Stack() {
      Canvas(this.ctx)
        .width('100%')
        .height('100%')
        .onReady(() => {
          this.canvasReady = true;
          hilog.info(DOMAIN, TAG, 'Canvas onReady');
          this.ensureBuffer();
          this.maybeStart();
        })
        .onAreaChange((_old, _newArea) => {
          this.ensureBuffer();
        })

      // RenderNode 宿主（GPU 渲染节点路径用，置顶）
      NodeContainer(this.nodeController)
        .width('100%')
        .height('100%')

      // HUD 覆盖层
      Column({ space: 10 }) {
        Text(this.hud)
          .fontSize(13)
          .fontColor('#FFFFFF')
          .backgroundColor('rgba(0,0,0,0.65)')
          .padding(8)
          .borderRadius(6)

        Row({ space: 8 }) {
          Button(`载体: ${CARRIER_NAMES[this.carrier]}`)
            .fontSize(12)
            .onClick(() => this.cycleCarrier())
          Button(this.modeDirty ? '局部' : '全屏')
            .fontSize(13)
            .onClick(() => this.modeDirty = !this.modeDirty)
          Button(this.running ? '停止' : '开始')
            .fontSize(13)
            .onClick(() => {
              this.running = !this.running;
              if (this.running) this.maybeStart();
            })
        }
      }
      .alignItems(HorizontalAlign.Start)
      .position({ x: 12, y: 12 })
    }
    .width('100%')
    .height('100%')
  }

  private cycleCarrier(): void {
    this.carrier = (this.carrier + 1) % CARRIER_NAMES.length;
    // 切到 RenderNode 时清空 Canvas 显示，避免重叠残留
    if (this.carrier === CARRIER_RENDERNODE) {
      this.ctx.clearRect(0, 0, this.pw, this.ph);
    }
  }

  /* ===================== 缓冲区管理 ===================== */

  private ensureBuffer(): void {
    if (!this.canvasReady) return;
    const pw = Math.round(this.ctx.width);
    const ph = Math.round(this.ctx.height);
    if (pw <= 0 || ph <= 0) return;
    if (pw === this.pw && ph === this.ph && this.imageDataA) return;
    this.pw = pw;
    this.ph = ph;
    hilog.info(DOMAIN, TAG, `allocate+capture ${pw}x${ph} = ${(pw * ph * 4 / 1024 / 1024).toFixed(1)}MB`);

    // 原生绘制渐变 A 并捕获到 ImageData A
    this.drawGradient(210);
    this.imageDataA = this.ctx.getImageData(0, 0, pw, ph);
    // 原生绘制渐变 B 并捕获到 ImageData B
    this.drawGradient(30);
    this.imageDataB = this.ctx.getImageData(0, 0, pw, ph);

    // 从 RGBA 缓冲区构建 GPU 友好的 PixelMap A/B
    const opts: image.InitializationOptions = {
      editable: true,
      pixelFormat: image.PixelMapFormat.RGBA_8888,
      size: { width: pw, height: ph }
    };
    try {
      this.pixelMapA = image.createPixelMapSync(this.imageDataA.data.buffer, opts);
      this.pixelMapB = image.createPixelMapSync(this.imageDataB.data.buffer, opts);
    } catch (e) {
      hilog.error(DOMAIN, TAG, 'createPixelMapSync failed: %{public}s', JSON.stringify(e));
    }

    this.nodeController.setFrame(pw, ph);
    this.maybeStart();
  }

  /* ===================== 渲染循环 ===================== */

  private maybeStart(): void {
    if (this.canvasReady && this.pw > 0 && this.imageDataA && this.imageDataB
        && !this.looping && this.running) {
      this.looping = true;
      this.scheduleNext();
    }
  }

  private scheduleNext(): void {
    if (!this.running) {
      this.looping = false;
      return;
    }
    // 10ms 间隔避免饿死 UI 线程事件队列（setTimeout(0) 紧凑循环会卡死主线程）
    setTimeout(() => this.tick(), 10);
  }

  private tick(): void {
    if (!this.running || !this.imageDataA || !this.imageDataB) {
      this.looping = false;
      return;
    }
    const t0 = Date.now();

    if (this.carrier === CARRIER_NATIVE) {
      // 原生绘制基线：每帧直接 createLinearGradient + fillRect 画渐变（GPU）
      this.drawGradient((Date.now() * 0.06) % 360);
    } else if (this.carrier === CARRIER_RENDERNODE) {
      // GPU 渲染节点：交替 A/B PixelMap，invalidate 触发 draw（drawing.Canvas GPU）
      const pm = (this.frameCounter & 1) ? this.pixelMapB : this.pixelMapA;
      this.nodeController.setPixelMap(pm);
    } else if (this.carrier === CARRIER_DRAWPIXELMAP) {
      // drawImage(PixelMap)：交替 A/B，GPU 纹理绘制
      const pm = (this.frameCounter & 1) ? this.pixelMapB : this.pixelMapA;
      if (pm) this.ctx.drawImage(pm, 0, 0);
    } else {
      // putImageData（软件基线）
      if (this.modeDirty) {
        this.advanceDirtyRect();
        const buf = (this.frameCounter & 1) ? this.imageDataB : this.imageDataA;
        this.ctx.putImageData(
          buf, this.dirtyX, this.dirtyY,
          this.dirtyX, this.dirtyY, this.kDirtyW, this.kDirtyH
        );
      } else {
        const buf = (this.frameCounter & 1) ? this.imageDataB : this.imageDataA;
        this.ctx.putImageData(buf, 0, 0);
      }
    }

    const dt = Date.now() - t0;
    this.frameCounter++;

    // FPS：1 秒滑动窗口
    const now = Date.now();
    this.frameTs.push(now);
    while (this.frameTs.length > 0 && now - this.frameTs[0] > 1000) {
      this.frameTs.shift();
    }
    this.fps = this.frameTs.length;
    this.frameMs = dt;

    // HUD 节流刷新
    if (now - this.lastHudTs > 200) {
      this.lastHudTs = now;
      const mb = (this.pw * this.ph * 4 / 1024 / 1024).toFixed(1);
      this.hud =
        `FPS: ${this.fps}    单帧: ${this.frameMs}ms    累计帧: ${this.frameCounter}\n` +
        `缓冲: ${this.pw}×${this.ph} = ${mb}MB\n` +
        `载体: ${CARRIER_NAMES[this.carrier]}${this.modeDirty ? ' / 局部' : ' / 全屏'}`;
    }

    this.scheduleNext();
  }

  /* ===================== 原生绘制 ===================== */

  private drawGradient(hue: number): void {
    const c0 = this.hsv(hue, 0.8, 0.95);
    const c1 = this.hsv(hue + 140, 0.85, 0.95);
    const g = this.ctx.createLinearGradient(0, 0, 0, this.ph);
    g.addColorStop(0, `rgb(${c0[0]},${c0[1]},${c0[2]})`);
    g.addColorStop(1, `rgb(${c1[0]},${c1[1]},${c1[2]})`);
    this.ctx.fillStyle = g;
    this.ctx.fillRect(0, 0, this.pw, this.ph);
  }

  private hsv(h: number, s: number, v: number): number[] {
    h = (h % 360 + 360) % 360;
    const c = v * s;
    const x = c * (1 - Math.abs((h / 60) % 2 - 1));
    const m = v - c;
    let r = 0, g = 0, b = 0;
    if (h < 60) { r = c; g = x; }
    else if (h < 120) { r = x; g = c; }
    else if (h < 180) { g = c; b = x; }
    else if (h < 240) { g = x; b = c; }
    else if (h < 300) { r = x; b = c; }
    else { r = c; b = x; }
    return [Math.round((r + m) * 255), Math.round((g + m) * 255), Math.round((b + m) * 255)];
  }

  private advanceDirtyRect(): void {
    this.dirtyX += this.dirtyDirX;
    this.dirtyY += this.dirtyDirY;
    if (this.dirtyX <= 0 || this.dirtyX + this.kDirtyW >= this.pw) {
      this.dirtyDirX = -this.dirtyDirX;
    }
    if (this.dirtyY <= 0 || this.dirtyY + this.kDirtyH >= this.ph) {
      this.dirtyDirY = -this.dirtyDirY;
    }
    this.dirtyX = Math.max(0, Math.min(this.pw - this.kDirtyW, this.dirtyX));
    this.dirtyY = Math.max(0, Math.min(this.ph - this.kDirtyH, this.dirtyY));
  }
}
