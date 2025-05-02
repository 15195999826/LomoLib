import pypinyin

# 示例中文字符串
chinese_text = "你好，世界！"

# 将中文字符串转换为拼音
pinyin_text = pypinyin.lazy_pinyin(chinese_text)

# 将拼音列表转换为字符串, 并且每个拼音的首字母大写
pinyin_string = ' '.join([s[0].upper() + s[1:] for s in pinyin_text])

print(pinyin_string)

# 打印python 版本
import sys
print(sys.version)