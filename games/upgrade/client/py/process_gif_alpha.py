#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from PIL import Image

def process_gif_with_alpha(gif_path, alpha_path, output_dir):
    """
    处理GIF图像和其对应的alpha通道GIF图像，将它们合并成带透明度的PNG图像
    
    参数:
    gif_path: 原始GIF图像路径
    alpha_path: 对应的alpha通道GIF图像路径
    output_dir: 输出目录路径
    """
    # 创建输出目录
    os.makedirs(output_dir, exist_ok=True)
    
    # 打开GIF图像
    gif_img = Image.open(gif_path)
    alpha_img = Image.open(alpha_path)
    
    # 获取图像信息
    frame_count = getattr(gif_img, "n_frames", 1)
    alpha_frame_count = getattr(alpha_img, "n_frames", 1)
    
    print(f"处理GIF: {os.path.basename(gif_path)}")
    print(f"帧数: {frame_count}")
    
    # 检查帧数是否匹配
    if frame_count != alpha_frame_count:
        print(f"警告: {gif_path} 和 {alpha_path} 的帧数不匹配 ({frame_count} vs {alpha_frame_count})")
        frame_count = min(frame_count, alpha_frame_count)
    
    # 处理每一帧
    for i in range(frame_count):
        # 提取GIF帧
        gif_img.seek(i)
        gif_frame = gif_img.convert('RGB')
        
        # 提取alpha帧
        alpha_img.seek(i)
        alpha_frame = alpha_img.convert('L')
        
        # 创建RGBA图像
        rgba_frame = gif_frame.copy()
        rgba_frame.putalpha(alpha_frame)
        
        # 保存为PNG
        output_path = os.path.join(output_dir, f"{i:03d}.png")
        rgba_frame.save(output_path, 'PNG')
        print(f"已保存帧: {output_path}")
    
    print(f"完成处理: {gif_path} -> {output_dir}")

def process_all_gifs(input_dir, output_base_dir):
    """
    处理目录中的所有GIF图像对
    
    参数:
    input_dir: 包含GIF图像的输入目录
    output_base_dir: 输出目录的基础路径
    """
    # 获取所有GIF文件
    gif_files = [f for f in os.listdir(input_dir) if f.endswith('.gif') and not f.endswith('A.gif')]
    
    for gif_file in gif_files:
        # 构造基本文件名（不带.gif扩展名）
        base_name = gif_file[:-4]  # 移除.gif扩展名
        alpha_file = base_name + 'A.gif'
        
        # 检查alpha文件是否存在
        gif_path = os.path.join(input_dir, gif_file)
        alpha_path = os.path.join(input_dir, alpha_file)
        
        if os.path.exists(alpha_path):
            # 构造输出目录路径
            output_dir = os.path.join(output_base_dir, base_name)
            process_gif_with_alpha(gif_path, alpha_path, output_dir)
        else:
            print(f"警告: 找不到对应的alpha文件 {alpha_path}")

def main():
    # 定义输入和输出目录
    input_dir = 'uires/Sprite'
    output_base_dir = 'uires/Sprite_png'
    
    # 处理所有GIF图像对
    process_all_gifs(input_dir, output_base_dir)
    print("所有GIF图像处理完成!")

if __name__ == '__main__':
    main()