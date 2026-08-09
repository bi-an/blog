import sys
from collections import defaultdict

def count_chinese(text):
    char_count = {}
    for char in text:
        if '\u4e00' <= char <= '\u9fff':
            char_count[char] = char_count.get(char, 0) + 1
    freq_dict = defaultdict(list)
    for ch, cnt in char_count.items():
        freq_dict[cnt].append(ch)
    sorted_freq = sorted(freq_dict.keys(), reverse=True)
    return sorted_freq, freq_dict, sum(char_count.values()), len(char_count)


def print_result(sorted_freq, freq_dict, total, distinct):
    print("\n===== 汉字统计（按频次分组） =====")
    # 表头
    print("频率\t汉字列表")
    print("-" * 40)
    for freq in sorted_freq:
        chars = "、".join(freq_dict[freq])
        print(f"{freq}\t{chars}")
    print(f"\n不重复汉字数：{distinct}")
    print(f"汉字总数量：{total}")

    # 保存文件，带表头
    with open("output.txt", "w", encoding="utf-8") as fout:
        fout.write("频率\t汉字列表\n")
        fout.write("-" * 40 + "\n")
        for freq in sorted_freq:
            chars = "、".join(freq_dict[freq])
            fout.write(f"{freq}\t{chars}\n")
        fout.write(f"\n不重复汉字数：{distinct}\n")
        fout.write(f"汉字总数量：{total}\n")
    print("\n✅结果已保存至 output.txt")


def main():
    args = sys.argv[1:]
    if len(args) == 0:
        print("请输入待统计中文文本，结束输入：Windows Ctrl+Z 回车；Linux/Mac Ctrl+D")
        content = sys.stdin.read()
        sorted_freq, freq_dict, total, distinct = count_chinese(content)
        print_result(sorted_freq, freq_dict, total, distinct)
    else:
        file_path = args[0]
        try:
            with open(file_path, "r", encoding="utf-8") as f:
                content = f.read()
            sorted_freq, freq_dict, total, distinct = count_chinese(content)
            print_result(sorted_freq, freq_dict, total, distinct)
        except FileNotFoundError:
            print(f"❌错误：找不到文件 {file_path}")
        except UnicodeDecodeError:
            print("❌错误：文件编码不是UTF‑8，请转换编码后重试")


if __name__ == "__main__":
    main()
