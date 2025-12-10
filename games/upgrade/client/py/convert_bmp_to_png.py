#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from PIL import Image

def convert_bmp_to_png_with_magenta_keying(input_path, output_path):
    """
    将24位BMP图像转换为32位PNG图像，使用洋红色(RGB:255,0,255)作为透明色
    
    参数:
    input_path: 输入BMP文件路径
    output_path: 输出PNG文件路径
    """
    # 打开BMP图像
    img = Image.open(input_path)
    
    # 转换为RGBA模式
    rgba_img = img.convert("RGBA")
    
    # 获取图像数据
    datas = rgba_img.getdata()
    
    # 创建新的像素数据列表
    new_data = []
    
    # 洋红色透明键颜色
    magenta_color = (255, 0, 255)
    
    # 处理每个像素
    for item in datas:
        # 如果像素是洋红色，则将其变为透明
        if item[:3] == magenta_color:
            new_data.append((255, 255, 255, 0))  # 透明像素
        else:
            new_data.append(item)
    
    # 应用新的像素数据
    rgba_img.putdata(new_data)
    
    # 保存为PNG
    rgba_img.save(output_path, "PNG")
    print(f"已转换: {os.path.basename(input_path)} -> {os.path.basename(output_path)}")

def process_directory(input_dir, output_dir):
    """
    处理目录中的所有BMP文件
    
    参数:
    input_dir: 输入目录路径
    output_dir: 输出目录路径
    """
    # 确保输出目录存在
    os.makedirs(output_dir, exist_ok=True)
    
    # 获取所有BMP文件
    bmp_files = [f for f in os.listdir(input_dir) if f.lower().endswith('.bmp')]
    
    # 处理每个BMP文件
    for bmp_file in bmp_files:
        input_path = os.path.join(input_dir, bmp_file)
        output_filename = os.path.splitext(bmp_file)[0] + '.png'
        output_path = os.path.join(output_dir, output_filename)
        
        print(f"处理: {bmp_file}")
        try:
            convert_bmp_to_png_with_magenta_keying(input_path, output_path)
        except Exception as e:
            print(f"处理 {bmp_file} 时出错: {e}")

def main():
    # 定义输入和输出目录
    input_dir = 'uires/bmp'
    output_dir = 'uires/bmp_png'
    
    # 处理图像
    process_directory(input_dir, output_dir)
    print("所有BMP图像转换完成!")

if __name__ == '__main__':
    main()