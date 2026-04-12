#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
从 SVG 图片生成 Windows ICO 和 macOS ICNS 图标文件

使用 svg2png.exe 将 SVG 转换为 PNG，然后生成图标格式。

依赖库:
    pip install Pillow

使用方法:
    python svg_to_icon_v2.py input.svg                    # 单个文件转换
    python svg_to_icon_v2.py input.svg -o output          # 指定输出路径（不含扩展名）
    python svg_to_icon_v2.py --batch ./icons/             # 批量转换目录下的所有 SVG
    python svg_to_icon_v2.py input.svg --size 512         # 指定 PNG 渲染尺寸

输出:
    - input.ico   : Windows ICO 图标（包含多尺寸：16, 32, 48, 64, 128, 256）
    - input.icns  : macOS ICNS 图标集（包含 16-1024 及 @2x 变体）
"""

import os
import sys
import argparse
import subprocess
from pathlib import Path
from PIL import Image


def get_script_dir():
    """获取脚本所在目录"""
    return Path(__file__).parent.resolve()


def svg_to_png(svg_path, png_path, size=1024):
    """
    使用 svg2png.exe 将 SVG 转换为 PNG
    
    Args:
        svg_path: SVG 文件路径
        png_path: 输出 PNG 路径
        size: 渲染尺寸（宽和高相同）
    
    Returns:
        bool: 是否成功
    """
    try:
        # 构建命令
        script_dir = get_script_dir()
        svg2png_exe = script_dir / 'svg2png.exe'
        
        if not svg2png_exe.exists():
            print(f"  ✗ 错误: 找不到 svg2png.exe: {svg2png_exe}")
            return False
        
        # 执行命令: svg2png.exe -i input.svg -s 1024 -o output.png
        cmd = [
            str(svg2png_exe),
            '-i', str(svg_path),
            '-s', str(size),
            '-o', str(png_path)
        ]
        
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=30  # 30秒超时
        )
        
        if result.returncode == 0 and png_path.exists():
            print(f"  ✓ 使用 svg2png.exe 渲染 SVG -> PNG ({size}x{size})")
            return True
        else:
            print(f"  ✗ svg2png.exe 执行失败")
            if result.stderr:
                print(f"     错误信息: {result.stderr.strip()}")
            return False
            
    except subprocess.TimeoutExpired:
        print(f"  ✗ svg2png.exe 执行超时（30秒）")
        return False
    except Exception as e:
        print(f"  ✗ SVG 转 PNG 失败: {e}")
        return False


def generate_ico_from_multiple_png(svg_path, output_path):
    """
    从 SVG 直接渲染多个尺寸的 PNG，然后组合成 ICO
    
    Args:
        svg_path: SVG 文件路径
        output_path: 输出 ICO 路径
    
    Returns:
        bool: 是否成功
    """
    try:
        from PIL import Image
        import tempfile
        import shutil
        import struct
        import io
        
        # ICO 需要的尺寸
        ico_sizes = [16, 32, 48, 64, 128, 256]
        
        # 临时目录存放不同尺寸的 PNG
        temp_dir = Path(tempfile.mkdtemp())
        png_files = []
        
        try:
            # 为每个尺寸直接从 SVG 渲染
            print(f"  正在从 SVG 渲染 {len(ico_sizes)} 个尺寸...")
            for size in ico_sizes:
                png_path = temp_dir / f"icon_{size}.png"
                if svg_to_png(svg_path, png_path, size):
                    png_files.append((size, png_path))
                else:
                    print(f"  ✗ 渲染 {size}x{size} 失败")
                    return False
            
            # 手动构建 ICO 文件
            # ICO 文件格式:
            # - 6 字节文件头
            # - 每个图像 16 字节目录项
            # - 图像数据（PNG 格式）
            
            icon_dir_header = struct.pack('<HHH', 
                0,      # 保留，必须为 0
                1,      # 图像类型：1 = ICO
                len(png_files)  # 图像数量
            )
            
            icon_dir_entries = b''
            image_data_list = []
            offset = 6 + (16 * len(png_files))  # 文件头 + 所有目录项的大小
            
            for size, png_path in sorted(png_files):
                with open(png_path, 'rb') as f:
                    png_data = f.read()
                
                # PNG 图像的宽度和高度
                width = size
                height = size
                
                # ICO 目录项：如果尺寸为 256，存储为 0
                icon_width = 0 if width == 256 else width
                icon_height = 0 if height == 256 else height
                
                entry = struct.pack('<BBBBHHII',
                    icon_width,     # 宽度
                    icon_height,    # 高度
                    0,              # 颜色数（0 = 无调色板）
                    0,              # 保留
                    1,              # 颜色平面
                    32,             # 每像素位数
                    len(png_data),  # 图像数据大小
                    offset          # 数据偏移量
                )
                
                icon_dir_entries += entry
                image_data_list.append(png_data)
                offset += len(png_data)
            
            # 写入 ICO 文件
            with open(output_path, 'wb') as f:
                f.write(icon_dir_header)
                f.write(icon_dir_entries)
                for data in image_data_list:
                    f.write(data)
            
            print(f"✓ 成功生成 ICO 文件（{len(png_files)} 个尺寸）: {output_path}")
            
            # 验证生成的 ICO
            try:
                with Image.open(output_path) as ico_verify:
                    frame_count = getattr(ico_verify, 'n_frames', 1)
                    print(f"  验证: ICO 包含 {frame_count} 个帧")
                    
                    # 显示所有尺寸
                    for i in range(frame_count):
                        ico_verify.seek(i)
                        print(f"    - 帧 {i+1}: {ico_verify.size[0]}x{ico_verify.size[1]}")
            except Exception as e:
                print(f"  警告: 验证失败 - {e}")
            
            return True
                
        finally:
            # 清理临时目录和文件
            try:
                shutil.rmtree(temp_dir, ignore_errors=True)
            except:
                pass
                
    except Exception as e:
        print(f"✗ 生成 ICO 文件失败: {e}")
        import traceback
        traceback.print_exc()
        return False


def generate_ico(png_path, output_path):
    """
    生成 Windows ICO 文件（旧方法，保留作为备选）
    
    Args:
        png_path: PNG 源文件路径
        output_path: 输出 ICO 路径
    
    Returns:
        bool: 是否成功
    """
    try:
        img = Image.open(png_path)
        
        # ICO 支持的多尺寸
        ico_sizes = [(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]
        
        # 保存为 ICO
        img.save(output_path, format='ICO', sizes=ico_sizes)
        print(f"✓ 成功生成 ICO 文件: {output_path}")
        return True
        
    except Exception as e:
        print(f"✗ 生成 ICO 文件失败: {e}")
        return False


def generate_icns_from_multiple_png(svg_path, output_path):
    """
    从 SVG 直接渲染多个尺寸的 PNG，然后组合成 ICNS
    
    Args:
        svg_path: SVG 文件路径
        output_path: 输出 ICNS 路径
    
    Returns:
        bool: 是否成功
    """
    if sys.platform != 'darwin':
        print(f"⚠ 警告: 当前平台 ({sys.platform}) 无法生成 ICNS 文件")
        print(f"  提示: 请将生成的 PNG 文件转移到 macOS 上使用 iconutil 命令转换")
        return False
    
    try:
        import tempfile
        
        # ICNS 需要的尺寸（标准 + @2x）
        icns_sizes_config = [
            (16, False), (16, True),   # 16x16, 32x32@2x
            (32, False), (32, True),   # 32x32, 64x64@2x
            (64, False),               # 64x64
            (128, False), (128, True), # 128x128, 256x256@2x
            (256, False),              # 256x256
            (512, False), (512, True), # 512x512, 1024x1024@2x
        ]
        
        with tempfile.TemporaryDirectory() as temp_dir:
            iconset_dir = Path(temp_dir) / 'icon.iconset'
            iconset_dir.mkdir()
            
            # 为每个尺寸直接从 SVG 渲染
            print(f"  正在从 SVG 渲染 {len(icns_sizes_config)} 个尺寸...")
            success_count = 0
            
            for size, is_2x in icns_sizes_config:
                render_size = size * 2 if is_2x else size
                
                if is_2x:
                    png_name = f'icon_{size}x{size}@2x.png'
                else:
                    png_name = f'icon_{size}x{size}.png'
                
                png_path = iconset_dir / png_name
                
                if svg_to_png(svg_path, png_path, render_size):
                    success_count += 1
                else:
                    print(f"  ✗ 渲染 {png_name} 失败")
            
            if success_count == 0:
                print(f"✗ 没有成功渲染任何尺寸")
                return False
            
            print(f"  ✓ 成功渲染 {success_count}/{len(icns_sizes_config)} 个尺寸")
            
            # 调用 iconutil
            result = subprocess.run([
                'iconutil', '-c', 'icns',
                str(iconset_dir),
                '-o', str(output_path)
            ], capture_output=True, text=True)
            
            if result.returncode == 0:
                print(f"✓ 成功生成 ICNS 文件: {output_path}")
                return True
            else:
                print(f"✗ iconutil 失败: {result.stderr}")
                return False
                
    except Exception as e:
        print(f"✗ 生成 ICNS 文件失败: {e}")
        import traceback
        traceback.print_exc()
        return False


def generate_icns(png_path, output_path):
    """
    生成 macOS ICNS 文件（旧方法，保留作为备选）
    
    Args:
        png_path: PNG 源文件路径
        output_path: 输出 ICNS 路径
    
    Returns:
        bool: 是否成功
    """
    try:
        img = Image.open(png_path)
        
        # macOS 平台：使用 iconutil
        if sys.platform == 'darwin':
            import tempfile
            
            with tempfile.TemporaryDirectory() as temp_dir:
                iconset_dir = Path(temp_dir) / 'icon.iconset'
                iconset_dir.mkdir()
                
                # ICNS 需要的尺寸
                icns_sizes = [16, 32, 64, 128, 256, 512, 1024]
                
                for size in icns_sizes:
                    # 标准尺寸
                    resized = img.resize((size, size), Image.Resampling.LANCZOS)
                    resized.save(iconset_dir / f'icon_{size}x{size}.png')
                    
                    # @2x 尺寸（Retina）
                    if size <= 256:
                        resized_2x = img.resize((size * 2, size * 2), Image.Resampling.LANCZOS)
                        resized_2x.save(iconset_dir / f'icon_{size}x{size}@2x.png')
                
                # 调用 iconutil
                result = subprocess.run([
                    'iconutil', '-c', 'icns',
                    str(iconset_dir),
                    '-o', str(output_path)
                ], capture_output=True, text=True)
                
                if result.returncode == 0:
                    print(f"✓ 成功生成 ICNS 文件: {output_path}")
                    return True
                else:
                    print(f"✗ iconutil 失败: {result.stderr}")
                    return False
        else:
            # 非 macOS 平台
            print(f"⚠ 警告: 当前平台 ({sys.platform}) 无法生成 ICNS 文件")
            print(f"  提示: 请将生成的 PNG 文件转移到 macOS 上使用 iconutil 命令转换")
            print(f"  命令: iconutil -c icns icon.iconset -o {output_path}")
            return False
            
    except Exception as e:
        print(f"✗ 生成 ICNS 文件失败: {e}")
        return False


def process_svg(svg_path, output_base=None, render_size=1024):
    """
    处理单个 SVG 文件，生成 ICO 和 ICNS
    
    Args:
        svg_path: SVG 文件路径
        output_base: 输出文件基础路径（不含扩展名），默认为 SVG 文件名
        render_size: SVG 渲染为 PNG 的尺寸（仅用于兼容性，新方法会忽略此参数）
    
    Returns:
        bool: 是否成功
    """
    svg_path = Path(svg_path)
    
    if not svg_path.exists():
        print(f"✗ 错误: 文件不存在: {svg_path}")
        return False
    
    if svg_path.suffix.lower() != '.svg':
        print(f"✗ 错误: 不是 SVG 文件: {svg_path}")
        return False
    
    # 确定输出路径
    if output_base:
        output_base = Path(output_base)
    else:
        output_base = svg_path.with_suffix('')
    
    print(f"\n处理: {svg_path.name}")
    print(f"输出基础路径: {output_base}")
    
    try:
        # 步骤 1: 生成 ICO（从 SVG 直接渲染多个尺寸）
        print(f"步骤 1/2: 生成 ICO（从 SVG 渲染 6 个尺寸）")
        ico_path = output_base.with_suffix('.ico')
        generate_ico_from_multiple_png(svg_path, ico_path)
        
        # 步骤 2: 生成 ICNS（从 SVG 直接渲染多个尺寸）
        print(f"步骤 2/2: 生成 ICNS（从 SVG 渲染 10 个尺寸）")
        icns_path = output_base.with_suffix('.icns')
        generate_icns_from_multiple_png(svg_path, icns_path)
        
        return True
        
    except Exception as e:
        print(f"✗ 处理 SVG 失败: {e}")
        import traceback
        traceback.print_exc()
        return False


def batch_process(input_dir, output_dir=None, render_size=1024):
    """
    批量处理目录下的所有 SVG 文件
    
    Args:
        input_dir: 输入目录
        output_dir: 输出目录，默认为输入目录
        render_size: SVG 渲染尺寸
    
    Returns:
        int: 成功处理的文件数量
    """
    input_dir = Path(input_dir)
    
    if not input_dir.exists() or not input_dir.is_dir():
        print(f"✗ 错误: 目录不存在: {input_dir}")
        return 0
    
    if output_dir:
        output_dir = Path(output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
    else:
        output_dir = input_dir
    
    # 查找所有 SVG 文件
    svg_files = list(input_dir.glob('*.svg')) + list(input_dir.glob('*.SVG'))
    
    if not svg_files:
        print(f"⚠ 目录中没有找到 SVG 文件: {input_dir}")
        return 0
    
    print(f"找到 {len(svg_files)} 个 SVG 文件")
    
    success_count = 0
    for svg_file in sorted(svg_files):
        output_base = output_dir / svg_file.stem
        if process_svg(svg_file, output_base, render_size):
            success_count += 1
    
    print(f"\n{'='*60}")
    print(f"批量处理完成: 成功 {success_count}/{len(svg_files)} 个文件")
    return success_count


def main():
    parser = argparse.ArgumentParser(
        description='从 SVG 图片生成 Windows ICO 和 macOS ICNS 图标（使用 svg2png.exe）',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  python svg_to_icon_v2.py icon.svg                  # 单个文件转换
  python svg_to_icon_v2.py icon.svg -o myapp         # 指定输出名称
  python svg_to_icon_v2.py --batch ./icons/          # 批量转换
  python svg_to_icon_v2.py icon.svg --size 512       # 指定渲染尺寸
        """
    )
    
    parser.add_argument('input', help='SVG 文件路径或目录（配合 --batch 使用）')
    parser.add_argument('-o', '--output', help='输出文件基础路径（不含扩展名），仅在单文件模式下有效')
    parser.add_argument('--batch', action='store_true', help='批量处理模式：处理目录下所有 SVG 文件')
    parser.add_argument('--size', type=int, default=1024, help='SVG 渲染为 PNG 的基础尺寸（默认: 1024）')
    
    args = parser.parse_args()
    
    # 验证参数
    if args.size < 16 or args.size > 4096:
        print("✗ 错误: 渲染尺寸必须在 16-4096 之间")
        sys.exit(1)
    
    # 检查 svg2png.exe 是否存在
    script_dir = get_script_dir()
    svg2png_exe = script_dir / 'svg2png.exe'
    if not svg2png_exe.exists():
        print(f"✗ 错误: 找不到 svg2png.exe: {svg2png_exe}")
        print(f"  请确保 svg2png.exe 与脚本在同一目录")
        sys.exit(1)
    
    # 执行处理
    if args.batch:
        # 批量模式
        success_count = batch_process(args.input, args.output, args.size)
        if success_count == 0:
            sys.exit(1)
    else:
        # 单文件模式
        if not process_svg(args.input, args.output, args.size):
            sys.exit(1)
    
    print("\n完成!")


if __name__ == "__main__":
    main()
