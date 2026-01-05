#!/usr/bin/env python3
import re
import sys
from pathlib import Path

def fix_set_cell_chessman(file_path):
    """Fix SetCellChessman calls that are missing the 4th parameter"""
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Pattern: SetCellChessman(ptCell, OR_RANK, SIDE_DIR) without the 4th parameter
    # Replace with: SetCellChessman(ptCell, OR_RANK, SIDE_DIR, TRUE)
    pattern = r'SetCellChessman\(([^,]+),\s*([^,]+),\s*(SIDE_\w+)\)(?!\s*,\s*TRUE)'
    replacement = r'SetCellChessman(\1, \2, \3, TRUE)'
    
    new_content = re.sub(pattern, replacement, content)
    
    if new_content != content:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(new_content)
        return True
    return False

# Fix all test files
test_dir = Path('d:/work/soui4/games/junqi/test')
for test_file in test_dir.glob('*Test.cpp'):
    if fix_set_cell_chessman(str(test_file)):
        print(f"Fixed {test_file.name}")
    else:
        print(f"No changes needed for {test_file.name}")
