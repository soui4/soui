import os
from PIL import Image

def combine_chess_pieces():
    # 定义图片文件夹路径
    folder_path = r"D:\work\soui4\games\cnchess\client\def_theme\combined_chess"
    
    # 检查文件夹是否存在
    if not os.path.exists(folder_path):
        print(f"文件夹 {folder_path} 不存在")
        return
    
    # 定义棋子顺序和对应的文件名模式
    red_order = [
        ("帅", "qipan_hong_shuai"),
        ("士", "qipan_hong_shi"),
        ("相", "qipan_hong_xiang"),
        ("马", "qipan_hong_ma"),
        ("车", "qipan_hong_che"),
        ("炮", "qipan_hong_pao"),
        ("兵", "qipan_hong_bing")
    ]
    
    black_order = [
        ("帅", "qipan_hei_shuai"),
        ("士", "qipan_hei_shi"),
        ("相", "qipan_hei_xiang"),
        ("马", "qipan_hei_ma"),
        ("车", "qipan_hei_che"),
        ("炮", "qipan_hei_pao"),
        ("兵", "qipan_hei_bing")
    ]
    
    # 收集所有图片
    red_images = []
    black_images = []
    
    print("正在加载红棋图片...")
    for name, prefix in red_order:
        file_name = f"{prefix}_combined.png"
        file_path = os.path.join(folder_path, file_name)
        if os.path.exists(file_path):
            img = Image.open(file_path)
            red_images.append(img)
            print(f"已加载: {file_name}")
        else:
            print(f"警告: 找不到文件 {file_name}")
    
    print("\n正在加载黑棋图片...")
    for name, prefix in black_order:
        file_name = f"{prefix}_combined.png"
        file_path = os.path.join(folder_path, file_name)
        if os.path.exists(file_path):
            img = Image.open(file_path)
            black_images.append(img)
            print(f"已加载: {file_name}")
        else:
            print(f"警告: 找不到文件 {file_name}")
    
    if not red_images and not black_images:
        print("没有找到任何图片文件")
        return
    
    # 计算最终图像尺寸
    max_width = 0
    total_height = 0
    
    for img in red_images + black_images:
        max_width = max(max_width, img.width)
        total_height += img.height
    
    # 创建新的图像
    combined_image = Image.new('RGBA', (max_width, total_height))
    
    # 粘贴红棋图片
    y_offset = 0
    print("\n正在组合红棋图片...")
    for i, img in enumerate(red_images):
        x_offset = (max_width - img.width) // 2  # 居中对齐
        combined_image.paste(img, (x_offset, y_offset))
        y_offset += img.height
        print(f"已粘贴红{red_order[i][0]}")
    
    # 粘贴黑棋图片
    print("\n正在组合黑棋图片...")
    for i, img in enumerate(black_images):
        x_offset = (max_width - img.width) // 2  # 居中对齐
        combined_image.paste(img, (x_offset, y_offset))
        y_offset += img.height
        print(f"已粘贴黑{black_order[i][0]}")
    
    # 保存最终图像
    output_path = os.path.join(folder_path, "combined_chess_pieces.png")
    combined_image.save(output_path, "PNG")
    print(f"\n已完成！图片已保存至: {output_path}")
    print(f"图像尺寸: {combined_image.width} x {combined_image.height}")

if __name__ == "__main__":
    combine_chess_pieces()