#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Simple BMP fixer for test1 failure fix.
Usage:
  python bmp_fix.py -d /path/to/dir/
"""
import os
import sys
import struct
import argparse

def read_u16(b, offset):
    return struct.unpack_from('<H', b, offset)[0]

def read_u32(b, offset):
    return struct.unpack_from('<I', b, offset)[0]

def read_s32(b, offset):
    return struct.unpack_from('<i', b, offset)[0]

def gather_bmp_files(root):
    files = []
    if os.path.isfile(root):
        if root.lower().endswith('.bmp'):
            files.append(root)
    else:
        for dirpath, dirnames, filenames in os.walk(root):
            for fn in filenames:
                if fn.lower().endswith('.bmp'):
                    files.append(os.path.join(dirpath, fn))
    return files

def compute_data_size_per_line(width, bpp):
    return ((width * bpp + 31) >> 3) & (~0x3)

def parse_bmp_header(path):
    with open(path, 'rb') as f:
        header = f.read(54)
        if len(header) < 14 or header[0:2] != b'BM':
            raise ValueError("not a standard BMP")
        bfSize = read_u32(header, 2)
        bfOffBits = read_u32(header, 10)
        if len(header) < 54:
            raise ValueError("unsupported or truncated BMP header")
        biSize = read_u32(header, 14)
        if biSize < 40:
            raise ValueError("unsupported BITMAPINFOHEADER (biSize < 40)")
        width = read_s32(header, 18)
        height = read_s32(header, 22)
        bits_per_pixel = read_u16(header, 28)
        biCompression = read_u32(header, 30)
        biSizeImage = read_u32(header, 34)
        return {
            'bfSize': bfSize,
            'bfOffBits': bfOffBits,
            'biSize': biSize,
            'width': width,
            'height': height,
            'bits_per_pixel': bits_per_pixel,
            'biCompression': biCompression,
            'biSizeImage': biSizeImage,
        }

def update_u32_in_file(path, offset, value):
    with open(path, 'r+b') as f:
        f.seek(offset)
        f.write(struct.pack('<I', value))

def truncate_and_fix(path, bfOffBits, raw_data_size, new_bfSize):
    new_file_size = new_bfSize
    with open(path, 'r+b') as f:
        f.truncate(new_file_size)
    update_u32_in_file(path, 2, new_bfSize)
    # update biSizeImage at offset 34 if header is standard (we assume it is)
    try:
        update_u32_in_file(path, 34, raw_data_size)
    except Exception:
        # if cannot update, ignore; truncation already done
        pass

def process_bmp(path):
    try:
        info = parse_bmp_header(path)
    except Exception:
        return False  # skip malformed or unsupported BMPs silently

    bfSize = info['bfSize']
    bfOffBits = info['bfOffBits']
    width = info['width']
    height = info['height']
    bpp = info['bits_per_pixel']
    biCompression = info['biCompression']

    if biCompression != 0:
        return False  # skip compressed BMPs silently

    data_per_line = compute_data_size_per_line(width, bpp)
    raw_data_size = abs(height) * data_per_line
    compute_size = bfSize - bfOffBits

    if compute_size > raw_data_size:
        # print the three-line message specified by user
        print(f"[{path}]: can not pass Test1. Trying fix...")
        print(f"[{path}]: RawDataSize: {raw_data_size:#x}, File Caculated Data Size {compute_size:#x}")
        print(f"[{path}]: Resize RawDataSize to Caculated Data Size.")
        new_bfSize = bfOffBits + raw_data_size
        try:
            truncate_and_fix(path, bfOffBits, raw_data_size, new_bfSize)
            return True
        except Exception:
            return False
    return False

def main(argv):
    parser = argparse.ArgumentParser(description="Fix BMPs where bfSize - bfOffBits > RawDataSize")
    parser.add_argument('-d', '--dir', required=True, help='Directory or file to scan recursively')
    args = parser.parse_args(argv)

    target = args.dir
    if not os.path.exists(target):
        print(f"specified path does not exist: {target}", file=sys.stderr)
        sys.exit(2)

    bmp_files = gather_bmp_files(target)
    scanned = len(bmp_files)
    fixed = 0

    for p in bmp_files:
        try:
            if process_bmp(p):
                fixed += 1
        except Exception:
            # ignore individual file errors and continue
            continue

    print(f"scanned {scanned} files, {fixed} of bmps fixed.")

if __name__ == '__main__':
    main(sys.argv[1:])
