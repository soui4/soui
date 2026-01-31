#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
生成Windows ICO和macOS ICNS图标文件

使用方法:
python generate_icons.py input.png

输入:
input.png - 256x256的PNG图片

输出:
- input.ico - Windows ICO图标文件
- input.icns - macOS ICNS图标文件
"""

import os
import sys
import tempfile
import subprocess
from PIL import Image

def generate_ico(input_path, output_path):
    """生成Windows ICO文件"""
    try:
        # 打开输入图片
        img = Image.open(input_path)
        
        # 确保图片尺寸是256x256
        if img.size != (256, 256):
            print(f"警告: 输入图片尺寸不是256x256，当前尺寸: {img.size}")
            img = img.resize((256, 256), Image.Resampling.LANCZOS)
        
        # 保存为ICO文件
        img.save(output_path, format='ICO', sizes=[(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
        print(f"成功生成ICO文件: {output_path}")
        return True
    except Exception as e:
        print(f"生成ICO文件失败: {e}")
        return False

def generate_icns(input_path, output_path):
    """生成macOS ICNS文件"""
    try:
        # 创建临时目录
        with tempfile.TemporaryDirectory() as temp_dir:
            # 打开输入图片
            img = Image.open(input_path)
            
            # 确保图片尺寸是256x256
            if img.size != (256, 256):
                print(f"警告: 输入图片尺寸不是256x256，当前尺寸: {img.size}")
                img = img.resize((256, 256), Image.Resampling.LANCZOS)
            
            # 生成不同尺寸的PNG图片
            sizes = [16, 32, 64, 128, 256, 512, 1024]
            for size in sizes:
                # 生成标准尺寸
                resized = img.resize((size, size), Image.Resampling.LANCZOS)
                resized.save(os.path.join(temp_dir, f"icon_{size}x{size}.png"))
                
                # 生成@2x尺寸
                if size <= 256:
                    resized_2x = img.resize((size*2, size*2), Image.Resampling.LANCZOS)
                    resized_2x.save(os.path.join(temp_dir, f"icon_{size}x{size}@2x.png"))
            
            # 使用iconutil命令生成ICNS文件 (仅在macOS上可用)
            if sys.platform == 'darwin':
                # 创建iconset目录
                iconset_dir = os.path.join(temp_dir, 'icon.iconset')
                os.makedirs(iconset_dir)
                
                # 复制图片到iconset目录，并重命名为iconutil期望的格式
                icon_names = {
                    16: 'icon_16x16.png',
                    32: 'icon_32x32.png',
                    64: 'icon_64x64.png',
                    128: 'icon_128x128.png',
                    256: 'icon_256x256.png',
                    512: 'icon_512x512.png',
                    1024: 'icon_1024x1024.png'
                }
                
                for size, name in icon_names.items():
                    src = os.path.join(temp_dir, f"icon_{size}x{size}.png")
                    dst = os.path.join(iconset_dir, name)
                    if os.path.exists(src):
                        os.rename(src, dst)
                
                # 处理@2x图片
                icon_names_2x = {
                    16: 'icon_16x16@2x.png',
                    32: 'icon_32x32@2x.png',
                    64: 'icon_64x64@2x.png',
                    128: 'icon_128x128@2x.png',
                    256: 'icon_256x256@2x.png'
                }
                
                for size, name in icon_names_2x.items():
                    src = os.path.join(temp_dir, f"icon_{size}x{size}@2x.png")
                    dst = os.path.join(iconset_dir, name)
                    if os.path.exists(src):
                        os.rename(src, dst)
                
                # 运行iconutil命令
                subprocess.run(['iconutil', '-c', 'icns', iconset_dir, '-o', output_path], check=True)
                print(f"成功生成ICNS文件: {output_path}")
                return True
            else:
                # 在非macOS平台上，提示用户
                print("警告: 非macOS平台，无法生成ICNS文件。请在macOS上运行此脚本以生成ICNS文件。")
                # 我们可以生成一个占位符文件，或者直接返回
                return False
                
    except Exception as e:
        print(f"生成ICNS文件失败: {e}")
        return False

def main():
    """主函数"""
    if len(sys.argv) != 2:
        print("使用方法: python generate_icons.py input.png")
        sys.exit(1)
    
    input_path = sys.argv[1]
    
    if not os.path.exists(input_path):
        print(f"错误: 输入文件不存在: {input_path}")
        sys.exit(1)
    
    if not input_path.lower().endswith('.png'):
        print("错误: 输入文件必须是PNG格式")
        sys.exit(1)
    
    # 获取输出文件名
    base_name = os.path.splitext(input_path)[0]
    ico_output = f"{base_name}.ico"
    icns_output = f"{base_name}.icns"
    
    # 生成ICO文件
    generate_ico(input_path, ico_output)
    
    # 生成ICNS文件
    generate_icns(input_path, icns_output)

if __name__ == "__main__":
    main()
