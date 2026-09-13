import { media } from '@kit.MediaKit';
import { fileIo as fs } from '@kit.CoreFileKit';
import { audio } from '@kit.AudioKit';

// Win32 SND_* 标志
const SND_SYNC = 0x0000;
const SND_ASYNC = 0x0001;
const SND_NODEFAULT = 0x0002;
const SND_LOOP = 0x0008;
const SND_PURGE = 0x0040;

// 系统提示音（MessageBeep）的合成参数：直接用 AudioRenderer 现场合成 PCM，
// 因此不需要在 HAP 里打包任何音频资源，也不依赖 TonePlayer（后者仅对系统应用开放）。
const BEEP_SAMPLE_RATE = 48000;   // 采样率（Hz），取 AudioSamplingRate.SAMPLE_RATE_48000
const BEEP_DURATION_MS = 120;     // 单次提示音时长（ms）
const BEEP_FREQ_HZ = 880;         // 提示音频率（Hz），接近 Windows 默认提示音音高
const BEEP_GAIN = 0.35;           // 峰值幅度（相对满量程），避免削顶与刺耳
const BEEP_FADE_MS = 5;           // 起止淡入淡出时长（ms），消除爆音

//
// 多实例音频播放器，支持同时播放多个声音
export class AudioPlayer {
  private playerSet: Set<media.AVPlayer> = new Set();

  //
// 播放 WAV 音频文件，支持多个声音同时播放。
// 空路径或 SND_PURGE 会停止所有正在播放的声音。
// pszSound 音频文件绝对路径
// fdwSound Win32 SND_* 标志
// true 表示播放请求已成功启动（同步检查通过），false 表示文件无效或参数错误
  public playSound(pszSound: string, fdwSound: number): boolean {
    // 处理停止请求：空路径或 SND_PURGE
    if (!pszSound || (fdwSound & SND_PURGE) !== 0) {
      this.stopAll();
      return true;
    }

    // 检查文件是否存在（同步）
    try {
      const stat = fs.statSync(pszSound);
      if (!stat.isFile()) {
        console.error(`[AudioPlayer] File not found: ${pszSound}`);
        return false;
      }
    } catch (err) {
      console.error(`[AudioPlayer] File stat error: ${err?.toString()}`);
      return false;
    }
    // 异步启动播放（不阻塞返回）
    this.startPlayback(pszSound, fdwSound).catch(() => {
      console.error(`[AudioPlayer] Playback failed for ${pszSound}}, unknown error`);
    });

    return true;
  }

  //
// 异步启动播放，创建 AVPlayer 并管理生命周期
  private async startPlayback(pszSound: string, fdwSound: number): Promise<void> {
    let avPlayer: media.AVPlayer | null = null;
    try {
      avPlayer = await media.createAVPlayer();
      avPlayer.url = pszSound;
      const isLoop = (fdwSound & SND_LOOP) !== 0;
      avPlayer.loop = isLoop;

      // 监听状态变化（含完成状态和日志）
      avPlayer.on('stateChange', (state: string) => {
        console.info(`[AudioPlayer] state: ${state}`);
        if (state === 'completed') {
          this.removePlayer(avPlayer);
        }
      });

      // 监听错误自动清理
      avPlayer.on('error', (err) => {
        console.error(`[AudioPlayer] Player error: ${err?.toString()}`);
        this.removePlayer(avPlayer);
      });

      await avPlayer.prepare();
      await avPlayer.play();

      // 播放成功后加入集合
      this.playerSet.add(avPlayer);
      console.info(`[AudioPlayer] Started: ${pszSound}`);
    } catch (err) {
      console.error(`[AudioPlayer] Start playback error: ${err?.toString()}`);
      if (avPlayer) {
        try {
          await avPlayer.release();
        } catch (releaseErr) {
          console.error(`[AudioPlayer] Release error: ${releaseErr?.toString()}`);
        }
      }
      // 重新抛出 Error 类型，以便上层 catch 处理
      throw new Error(err?.toString() || 'Unknown playback error');
    }
  }

  //
// 从集合中移除并释放播放器
  private async removePlayer(player: media.AVPlayer | null): Promise<void> {
    if (!player) return;
    if (this.playerSet.has(player)) {
      this.playerSet.delete(player);
      try {
        await player.stop();
        await player.release();
        console.info('[AudioPlayer] Player removed and released');
      } catch (err) {
        console.error(`[AudioPlayer] Remove/release error: ${err?.toString()}`);
      }
    }
  }

  //
// 停止所有正在播放的声音，释放所有播放器
  private async stopAll(): Promise<void> {
    if (this.playerSet.size === 0) return;
    const players = Array.from(this.playerSet);
    this.playerSet.clear();
    for (const player of players) {
      try {
        await player.stop();
        await player.release();
      } catch (err) {
        console.error(`[AudioPlayer] StopAll error: ${err?.toString()}`);
      }
    }
    console.info('[AudioPlayer] All players stopped');
  }

  //
// 播放系统提示音（对齐 Win32 MessageBeep）。
// 鸿蒙可直接调用的 TonePlayer 属系统接口（仅对系统应用开放），第三方应用不可用，
// 因此这里用公开的 AudioRenderer 现场合成一段短促正弦音，并用"通知"流播报
// （StreamUsage 枚举里没有 STREAM_USAGE_SYSTEM，提示音语义对应 STREAM_USAGE_NOTIFICATION；
//  该流不可用时退化为 STREAM_USAGE_GAME），音量/静音策略由系统统一管理。
// true 表示提示音请求已提交（异步播放，不等待播完）
  public playBeep(): boolean {
    this.startBeep().catch(() => {
      console.error('[AudioPlayer] Beep failed, unknown error');
    });
    return true;
  }

  //
// 异步合成并播放一次提示音
  private async startBeep(): Promise<void> {
    const streamInfo: audio.AudioStreamInfo = {
      samplingRate: audio.AudioSamplingRate.SAMPLE_RATE_48000,
      channels: audio.AudioChannel.CHANNEL_1,
      sampleFormat: audio.AudioSampleFormat.SAMPLE_FORMAT_S16LE,
      encodingType: audio.AudioEncodingType.ENCODING_TYPE_RAW
    };
    const renderer = await this.createBeepRenderer(streamInfo);
    if (renderer === null) {
      console.error('[AudioPlayer] Beep: no audio renderer available');
      return;
    }
    try {
      await renderer.start();
      await renderer.write(this.makeBeepBuffer());
      // write() 返回只表示 PCM 已交给渲染器，真正播完还需要一段时间，
      // 因此等提示音时长走完再停，避免把尾音截断。
      await new Promise<void>((resolve) => {
        setTimeout(() => {
          resolve();
        }, BEEP_DURATION_MS + 50);
      });
    } finally {
      try {
        await renderer.stop();
      } catch (stopErr) {
        console.error(`[AudioPlayer] Beep stop error: ${stopErr?.toString()}`);
      }
      await renderer.release();
    }
  }

  //
// 创建提示音专用的 AudioRenderer。优先用通知音流 STREAM_USAGE_NOTIFICATION(=7)，
// 注释原文即 "Notification usage"，正是 MessageBeep 对应的流类型，音量/静音/勿扰由系统统一管理；
// 该流不可用时退化为 STREAM_USAGE_GAME(=11)（游戏音效流走并发 Mix，不抢占其他应用的音频焦点），
// 保证提示音仍可闻。
// 注意：StreamUsage 里没有 STREAM_USAGE_SYSTEM，误用会直接编译失败（10505001）。
// 全部失败返回 null
  private async createBeepRenderer(
    streamInfo: audio.AudioStreamInfo): Promise<audio.AudioRenderer | null> {
    const usages: audio.StreamUsage[] = [
      audio.StreamUsage.STREAM_USAGE_NOTIFICATION,
      audio.StreamUsage.STREAM_USAGE_GAME
    ];
    for (let i = 0; i < usages.length; i++) {
      try {
        const rendererInfo: audio.AudioRendererInfo = { usage: usages[i], rendererFlags: 0 };
        return await audio.createAudioRenderer({ streamInfo: streamInfo, rendererInfo: rendererInfo });
      } catch (err) {
        console.warn(`[AudioPlayer] createAudioRenderer(usage=${usages[i]}) failed: ${err?.toString()}`);
      }
    }
    return null;
  }

  //
// 合成一段 16bit 单声道 PCM 提示音（正弦波 + 起止淡入淡出）
  private makeBeepBuffer(): ArrayBuffer {
    const total = Math.floor(BEEP_SAMPLE_RATE * BEEP_DURATION_MS / 1000);
    const fade = Math.max(1, Math.floor(BEEP_SAMPLE_RATE * BEEP_FADE_MS / 1000));
    const buf = new ArrayBuffer(total * 2);
    const view = new DataView(buf);
    for (let i = 0; i < total; i++) {
      // 两端各做 fade 个采样点的线性包络
      const env = Math.min(1, Math.min(i, total - 1 - i) / fade);
      const wave = Math.sin(2 * Math.PI * BEEP_FREQ_HZ * i / BEEP_SAMPLE_RATE);
      view.setInt16(i * 2, Math.round(wave * env * BEEP_GAIN * 32767), true);
    }
    return buf;
  }

  //
// 释放全部资源（应用退出时调用）
  public async release(): Promise<void> {
    await this.stopAll();
  }
}