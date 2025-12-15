#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import subprocess
from PIL import Image

def get_gif_frame_delays(gif_path):
    """
    从GIF文件中提取每一帧的延迟时间
    
    参数:
    gif_path: GIF文件路径
    
    返回:
    delays: 每一帧的延迟时间列表(毫秒)
    """
    delays = []
    try:
        gif_img = Image.open(gif_path)
        frame_count = getattr(gif_img, "n_frames", 1)
        
        for i in range(frame_count):
            gif_img.seek(i)
            # 获取帧的延迟时间(以毫秒为单位)
            delay = gif_img.info.get('duration', 100)  # 默认100ms
            delays.append(delay)
    except Exception as e:
        print(f"读取GIF延迟时间时出错 {gif_path}: {e}")
        # 返回默认延迟时间列表
        try:
            # 尝试获取帧数
            gif_img = Image.open(gif_path)
            frame_count = getattr(gif_img, "n_frames", 1)
            delays = [100] * frame_count  # 默认每帧100ms
        except:
            delays = [100]  # 至少一帧
    
    return delays

def create_apng_with_tool(apngasm_path, frame_dir, output_path, delays):
    """
    使用apngasm_gui.exe工具将PNG帧序列创建为APNG文件
    
    参数:
    apngasm_path: apngasm_gui.exe工具路径
    frame_dir: PNG帧序列目录
    output_path: 输出APNG文件路径
    delays: 每帧延迟时间列表(毫秒)
    """
    # 构造命令行参数
    # apngasm_gui.exe output.png frame1.png frame2.png ... [options]
    cmd = [apngasm_path, output_path]
    
    # 添加所有帧文件
    frame_files = sorted([f for f in os.listdir(frame_dir) if f.endswith('.png')])
    for frame_file in frame_files:
        cmd.append(os.path.join(frame_dir, frame_file))
    
    # 添加延迟参数
    # 对于每一帧添加延迟参数
    for i, delay in enumerate(delays):
        # apngasm使用帧数(百分之一秒)作为延迟单位
        delay_centisec = max(1, round(delay / 10))  # 最小为1 (0.01秒)
        cmd.append(f"-d{delay_centisec}")
    
    # 添加循环次数(0表示无限循环)
    cmd.append("-l0")
    
    try:
        # 执行命令
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        print(f"成功创建APNG: {output_path}")
        return True
    except subprocess.CalledProcessError as e:
        print(f"创建APNG失败: {e}")
        print(f"命令输出: {e.stdout}")
        print(f"错误信息: {e.stderr}")
        return False
    except Exception as e:
        print(f"执行命令时出错: {e}")
        return False

def process_all_sequences(sprite_dir, sprite_png_dir, output_dir, apngasm_path):
    """
    处理所有PNG帧序列目录，生成对应的APNG文件
    
    参数:
    sprite_dir: 原始Sprite目录(GIF文件)
    sprite_png_dir: PNG帧序列目录
    output_dir: APNG输出目录
    apngasm_path: apngasm_gui.exe工具路径
    """
    # 创建输出目录
    os.makedirs(output_dir, exist_ok=True)
    
    # 获取所有PNG帧序列目录
    frame_dirs = [d for d in os.listdir(sprite_png_dir) 
                  if os.path.isdir(os.path.join(sprite_png_dir, d))]
    
    for frame_dir in frame_dirs:
        print(f"\n处理帧序列: {frame_dir}")
        
        # 构造对应的GIF文件路径
        gif_path = os.path.join(sprite_dir, frame_dir + '.gif')
        if not os.path.exists(gif_path):
            print(f"警告: 找不到对应的GIF文件 {gif_path}")
            continue
        
        # 获取帧延迟时间
        delays = get_gif_frame_delays(gif_path)
        print(f"帧延迟时间: {delays}")
        
        # 构造输入和输出路径
        frame_dir_path = os.path.join(sprite_png_dir, frame_dir)
        apng_output_path = os.path.join(output_dir, frame_dir + '.png')
        
        # 创建APNG文件
        success = create_apng_with_tool(apngasm_path, frame_dir_path, apng_output_path, delays)
        if success:
            print(f"成功生成APNG: {apng_output_path}")
        else:
            print(f"生成APNG失败: {frame_dir}")

def main():
    # 定义路径
    sprite_dir = 'uires/Sprite'           # 原始Sprite目录(GIF文件)
    sprite_png_dir = 'uires/Sprite_png'   # PNG帧序列目录
    output_dir = 'uires/APNG'             # APNG输出目录
    apngasm_path = 'uires/apngasm_gui.exe' # apngasm工具路径
    
    # 检查apngasm工具是否存在
    if not os.path.exists(apngasm_path):
        print(f"错误: 找不到apngasm工具 {apngasm_path}")
        return
    
    # 处理所有帧序列
    process_all_sequences(sprite_dir, sprite_png_dir, output_dir, apngasm_path)
    print("\n所有APNG文件处理完成!")

if __name__ == '__main__':
    main()