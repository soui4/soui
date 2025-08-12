#!/usr/bin/env python3
"""
将PNG图片生成.icns图标文件的脚本
可以通过命令行参数传入PNG图片路径，如果不传则默认使用./pngs目录中的图片
"""

import os
import subprocess
import tempfile
import shutil
from pathlib import Path
import sys
import argparse


def create_icns_from_pngs(png_paths, output_path="./icon.icns"):
    """
    从PNG图片创建.icns图标文件
    
    Args:
        png_paths: PNG文件路径列表
        output_path: 输出的.icns文件路径
    """
    
    # 检查PNG文件是否存在
    png_files = []
    for png_path in png_paths:
        png_file = Path(png_path)
        if not png_file.exists():
            print(f"警告: {png_file} 文件不存在，已跳过")
            continue
        
        if not png_file.suffix.lower() == '.png':
            print(f"警告: {png_file} 不是PNG文件，已跳过")
            continue
            
        png_files.append(png_file)
    
    if not png_files:
        print("错误: 没有有效的PNG文件")
        return False
    
    print(f"找到 {len(png_files)} 个PNG文件:")
    for png in png_files:
        print(f"  - {png}")
    
    # 创建临时目录用于存放不同尺寸的图标
    with tempfile.TemporaryDirectory() as temp_dir:
        iconset_dir = Path(temp_dir) / "icon.iconset"
        iconset_dir.mkdir()
        
        # 为每个PNG文件生成不同尺寸的图标
        required_sizes = [16, 32, 64, 128, 256, 512]
        
        for png_file in png_files:
            print(f"\n处理 {png_file}...")
            
            # 获取原始图片信息
            try:
                result = subprocess.run([
                    "sips", "-g", "pixelWidth", "-g", "pixelHeight", str(png_file)
                ], capture_output=True, text=True, check=True)
                
                lines = result.stdout.strip().split('\n')
                width = int(lines[1].split()[-1])  # pixelWidth: XXX
                height = int(lines[2].split()[-1])  # pixelHeight: XXX
                
                print(f"  原始尺寸: {width}x{height}")
                
            except subprocess.CalledProcessError as e:
                print(f"  错误: 无法获取图片信息: {e}")
                continue
            except (IndexError, ValueError) as e:
                print(f"  错误: 无法解析图片尺寸: {e}")
                continue
            
            # 选择最接近的尺寸
            max_size = max(width, height)
            target_size = min(required_sizes, key=lambda x: abs(x - max_size))
            
            # 生成对应尺寸的图标
            icon_name = f"icon_{target_size}x{target_size}.png"
            if target_size > 128:
                icon_name = f"icon_{target_size}x{target_size}@2x.png"
                target_size //= 2
            
            icon_path = iconset_dir / icon_name
            
            try:
                subprocess.run([
                    "sips", "-z", str(target_size), str(target_size), 
                    str(png_file), "--out", str(icon_path)
                ], check=True)
                print(f"  生成 {icon_name}")
            except subprocess.CalledProcessError as e:
                print(f"  错误: 无法调整图片尺寸: {e}")
                continue
        
        # 检查是否至少生成了一个图标
        generated_icons = list(iconset_dir.glob("*.png"))
        if not generated_icons:
            print("错误: 没有成功生成任何图标")
            return False
        
        print(f"\n总共生成了 {len(generated_icons)} 个图标")
        
        # 使用iconutil创建.icns文件
        try:
            subprocess.run([
                "iconutil", "-c", "icns", str(iconset_dir), "-o", output_path
            ], check=True)
            print(f"\n成功创建图标文件: {output_path}")
            return True
        except subprocess.CalledProcessError as e:
            print(f"错误: 无法创建.icns文件: {e}")
            return False


def main():
    parser = argparse.ArgumentParser(description="将PNG图片转换为.icns图标文件")
    parser.add_argument("png_files", nargs="*", help="PNG文件路径列表")
    parser.add_argument("-o", "--output", default="./icon.icns", help="输出.icns文件路径 (默认: ./icon.icns)")
    
    args = parser.parse_args()
    
    # 如果没有提供PNG文件路径，则使用默认的./pngs目录
    if not args.png_files:
        pngs_dir = Path("./pngs")
        if pngs_dir.exists():
            args.png_files = [str(f) for f in pngs_dir.glob("*.png")]
            if not args.png_files:
                print("错误: ./pngs 目录中没有找到PNG文件")
                return False
        else:
            print("错误: 请提供PNG文件路径或确保./pngs目录存在")
            return False
    
    print(f"开始生成.icns图标文件...")
    print(f"输出路径: {args.output}")
    success = create_icns_from_pngs(args.png_files, args.output)
    if success:
        print("完成!")
    else:
        print("操作失败!")
        sys.exit(1)


if __name__ == "__main__":
    main()