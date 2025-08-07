from PIL import Image, ImageDraw, ImageFont
import os
import matplotlib.font_manager as fm
import shutil
import cv2

# 2. 输出目录
output_dir = "emoji_pngs"
os.makedirs(output_dir, exist_ok=True)

# 3. 要生成的表情 Unicode 列表（F601-F637）
emoji_codes = ["😶",
"🙂",
"😆",
"😂",
"😔",
"😠",
"😭",
"😍",
"😳",
"😯",
"😱",
"🤔",
"😉",
"😎",
"😌",
"🤤",
"😘",
"😏",
"😴",
"😜",
"🙄", chr(0x1f914), chr(0x1f92a), chr(0x266A)]  # 0xF601 ~ 0xF637

# 4. 逐个生成 PNG
for code in emoji_codes:
    char = code
    print(char)
    filename = f"{hex(ord(char))[3:]}.png"
    file = f"/Users/jecelyin/Downloads/twemoji-master/assets/72x72/{filename}"
    if not os.path.exists(file):
        print(f"不存在 {file}，跳过")
        continue

    img1 = cv2.imread(file, cv2.IMREAD_UNCHANGED)
    img1 = cv2.resize(img1, (44, 44), cv2.INTER_LINEAR)
    cv2.imwrite(os.path.join(output_dir, filename), img1)
    # shutil.copy(file, os.path.join(output_dir, filename))

print(f"生成完成！PNG 保存在 {output_dir}")