#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import json
from PIL import Image, ImageOps


# 定义使用alpha透明通道的图像配置信息
# 格式: {文件名: (子图数量, 是否水平排列)}
IMAGE_CONFIG_ALPHA = {

}

# 定义使用mask模式的图像配置信息
# 格式: {文件名: (子图数量, 是否水平排列)}
IMAGE_CONFIG_MASK = {
'重新开始.bmp': (5, True),
'停止播放.bmp': (5, True),
'自动播放.bmp': (5, True),
}

def convert_bmp_to_png_with_alpha(input_path, output_path, subimage_info=None, use_mask=False):
    """
    将包含透明通道的24位BMP图像转换为32位PNG图像
    最后一个子图作为透明通道应用于前面的所有子图，并将处理后的子图按原顺序合并到一个PNG文件中
    
    参数:
    input_path: 输入BMP文件路径
    output_path: 输出PNG文件路径
    subimage_info: (子图数量, 是否水平排列, 是否使用mask模式) 元组，如果为None则自动判断
    """
    # 打开图像
    img = Image.open(input_path)
    
    # 获取图像尺寸
    width, height = img.size
    
    # 获取子图信息
    if subimage_info:
        num_subimages, is_horizontal = subimage_info
        if is_horizontal:
            subimage_width = width // num_subimages
            subimage_height = height
        else:
            subimage_width = width
            subimage_height = height // num_subimages
    else:
        # 自动判断子图数量和排列方式
        # 假设至少有2个子图（1个图像+1个透明通道）
        # 通过宽高比判断排列方式
        if width > height:
            # 水平排列
            is_horizontal = True
            # 简单估计子图数量（至少2个）
            estimated_count = max(2, round(width / height))
            subimage_width = width // estimated_count
            subimage_height = height
            # 重新计算实际子图数量
            num_subimages = width // subimage_width
        else:
            # 垂直排列
            is_horizontal = False
            estimated_count = max(2, round(height / width))
            subimage_width = width
            subimage_height = height // estimated_count
            # 重新计算实际子图数量
            num_subimages = height // subimage_height
        
        # 默认不使用mask模式
        use_mask = False
    
    if num_subimages < 2:
        print(f"警告: {input_path} 中的图像似乎不包含透明通道")
        # 直接保存为PNG
        img.save(output_path, 'PNG')
        return
    
    print(f"处理图像: {os.path.basename(input_path)}, 子图数量: {num_subimages}, 水平排列: {is_horizontal}, Mask模式: {use_mask}")
    
    # 提取透明通道（最后一个子图）
    if is_horizontal:
        alpha_region = img.crop(((num_subimages - 1) * subimage_width, 0, width, height))
    else:
        alpha_region = img.crop((0, (num_subimages - 1) * subimage_height, width, height))
    
    # 处理透明通道
    if use_mask:
        # Mask模式：黑色代表不透明，白色代表透明，需要反色处理
        alpha_data = alpha_region.convert('L')
        alpha_data = ImageOps.invert(alpha_data)
    else:
        # 正常模式：直接使用透明通道
        alpha_data = alpha_region.convert('L')
    
    # 处理每个子图（除了透明通道）并存储在列表中
    processed_subimages = []
    for i in range(num_subimages - 1):
        # 提取子图
        if is_horizontal:
            subimage = img.crop((i * subimage_width, 0, (i + 1) * subimage_width, height))
            alpha_channel = alpha_data.crop((0, 0, subimage_width, height))
        else:
            subimage = img.crop((0, i * subimage_height, width, (i + 1) * subimage_height))
            alpha_channel = alpha_data.crop((0, 0, width, subimage_height))
        
        # 创建RGBA图像
        rgba_image = subimage.convert('RGBA')
        
        # 应用透明通道
        r, g, b, _ = rgba_image.split()
        rgba_image = Image.merge('RGBA', (r, g, b, alpha_channel))
        
        # 添加到处理后的子图列表
        processed_subimages.append(rgba_image)
        print(f"处理子图: {i}")
    
    # 将所有处理后的子图按原顺序合并到一个新的图像中
    if is_horizontal:
        # 水平排列
        merged_width = subimage_width * len(processed_subimages)
        merged_height = subimage_height
        merged_image = Image.new('RGBA', (merged_width, merged_height))
        
        for i, subimage in enumerate(processed_subimages):
            merged_image.paste(subimage, (i * subimage_width, 0))
    else:
        # 垂直排列
        merged_width = subimage_width
        merged_height = subimage_height * len(processed_subimages)
        merged_image = Image.new('RGBA', (merged_width, merged_height))
        
        for i, subimage in enumerate(processed_subimages):
            merged_image.paste(subimage, (0, i * subimage_height))
    
    # 保存合并后的图像
    merged_image.save(output_path, 'PNG')
    print(f"已保存合并图像: {output_path}")

def process_directory(input_dir, output_dir):
    """
    处理目录中的所有BMP文件
    """
    # 确保输出目录存在
    os.makedirs(output_dir, exist_ok=True)
    
    # 遍历输入目录中的所有文件
    for filename in os.listdir(input_dir):
        if filename.lower().endswith('.bmp'):
            input_path = os.path.join(input_dir, filename)
            output_filename = os.path.splitext(filename)[0] + '.png'
            output_path = os.path.join(output_dir, output_filename)
            
            print(f"\n处理: {input_path}")
            try:
                # 首先检查是否在alpha配置中
                if  filename in IMAGE_CONFIG_MASK:
                    subimage_info = IMAGE_CONFIG_MASK.get(filename)
                    convert_bmp_to_png_with_alpha(input_path, output_path, subimage_info, use_mask=True)
                
            except Exception as e:
                print(f"处理 {input_path} 时出错: {e}")

def main():
    # 定义输入和输出目录
    input_dir = 'D:\\work\\mycode(2003-2005)\\联众\\junqihun\\CLIENT\\jqhres936\\res'
    output_dir = 'D:/work/soui4/games/junqi/client/def_theme/png'
    
    # 处理图像
    process_directory(input_dir, output_dir)
    print("\n所有图像处理完成!")

if __name__ == '__main__':
    main()