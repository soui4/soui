import { media } from '@kit.MediaKit';
import { fileIo as fs } from '@kit.CoreFileKit';

// Win32 SND_* 标志
const SND_SYNC = 0x0000;
const SND_ASYNC = 0x0001;
const SND_NODEFAULT = 0x0002;
const SND_LOOP = 0x0008;
const SND_PURGE = 0x0040;

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
// 释放全部资源（应用退出时调用）
  public async release(): Promise<void> {
    await this.stopAll();
  }
}