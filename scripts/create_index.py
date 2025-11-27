import os
import struct
import re
from functools import cmp_to_key
import sys
import mutagen  # 用于获取音频文件元数据

# 支持的音频文件扩展名
AUDIO_EXTENSIONS = {'.mp3', '.wav', '.flac', '.ogg', '.m4a', '.aac'}

# 中文数字到阿拉伯数字的映射（用于自定义排序）
chinese_to_num = {
    "一": 1, "二": 2, "三": 3, "四": 4, "五": 5,
    "六": 6, "七": 7, "八": 8, "九": 9, "十": 10,
    "百": 100
}


def is_audio_file(filename):
  """检查文件是否是音频文件"""
  return os.path.splitext(filename)[1].lower() in AUDIO_EXTENSIONS


def get_audio_duration(filepath):
  """获取音频文件时长（秒）"""
  try:
    audio = mutagen.File(filepath)
    return int(audio.info.length)
  except:
    return 0


def extract_number_from_prefix(filename):
  """提取文件名中的数字部分"""
  match = re.match(r'^([0-9一二三四五六七八九十百]+)', filename)
  if not match:
    match = re.search(r'([0-9一二三四五六七八九十百]+)\.[^\.]+$', filename)
    if not match:
      return filename[0]

  prefix = match.group(1)
  sequeue = ''.join(str(chinese_to_num.get(char, char)) for char in prefix)

  if sequeue.isdigit():
    print(f"解析文件名序号: {filename}，结果：{sequeue}")
    return str(sequeue)
  print(f"无法解析文件名序号: {filename}，结果：{sequeue}")
  return sequeue


def traverse_for_audio(root_dir):
  """递归遍历目录寻找音频文件"""
  folders = []
  files = []

  files_txt_path = os.path.join(root_dir, "files.txt")
  
  if os.path.isfile(files_txt_path):
      print(f"检测到 files.txt，使用自定义列表: {files_txt_path}")
      with open(files_txt_path, "r", encoding="utf-8") as f:
          for line in f:
              line = line.strip()
              if not line or line.startswith("#"):  # 跳过空行和注释
                  continue
              if line.startswith("/"):  # 表示子文件夹（相对路径）
                  folders.append(line[1:])  # 去掉前导 "/"
              else:
                  files.append(line)
  else:
    with os.scandir(root_dir) as entries:
      for entry in entries:
        if entry.is_dir():
          folders.append(entry.name)
        elif entry.is_file() and not entry.name.startswith('.') and is_audio_file(entry.path):
          files.append(entry.name)
    folders.sort()
    files.sort(key=extract_number_from_prefix)
  index_path = os.path.join(root_dir, "audio_index.bin")
  with open(index_path, 'wb') as f:
    for folder in folders:
      traverse_for_audio(os.path.join(root_dir, folder))
      packed_data = struct.pack("b80sii",
                                False,  # is file
                                folder.encode('utf-8'),  # folder name
                                0, 0
                                )
      f.write(packed_data)
    for file_name in files:
      file = os.path.join(root_dir, file_name)
      duration = get_audio_duration(file)
      size = os.path.getsize(file)
      packed_data = struct.pack("b80sii",
                                True,    # is file
                                file_name.encode('utf-8'), # name
                                size, # file size
                                duration # duration
                                )
      f.write(packed_data)

    print(f"已创建音频索引: {index_path} (包含 {len(files)} 个音频文件)")


if __name__ == "__main__":
  # 安装依赖: pip install mutagen
  print("开始扫描音频文件并创建索引...")
  # 判断是否有命令行参数
  if len(sys.argv) > 1:
    scan_path = os.path.abspath(sys.argv[1])
    print(f"使用指定路径进行扫描: {scan_path}")
  else:
    scan_path = os.path.dirname(os.path.abspath(__file__))
    print("未指定路径，使用脚本所在目录进行扫描")
  
  traverse_for_audio(scan_path)
  print("音频索引创建完成")
