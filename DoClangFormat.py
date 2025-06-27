#todo
import subprocess
import os
import sys

# 配置需要检查的文件扩展名
FILE_EXTENSIONS = [".cpp", ".h", ".cc", ".hpp"]

# 遍历目录，找到所有符合条件的文件
def get_source_files(directory):
    source_files = []
    for root, _, files in os.walk(directory):
        for file in files:
            if any(file.endswith(ext) for ext in FILE_EXTENSIONS):
                source_files.append(os.path.join(root, file))
    return source_files

# 检查文件是否符合 clang-format 的格式
def check_clang_format(file):
    try:
        # 使用 clang-format --dry-run --Werror 检查格式
        subprocess.run(
            ["clang-format", "--dry-run", "--Werror", file],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        return True
    except subprocess.CalledProcessError as e:
        print(f"File {file} is not properly formatted.")
        return False

def main():
    # 默认从当前目录开始检查
    directory = "."
    if len(sys.argv) > 1:
        directory = sys.argv[1]

    # 查找所有源代码文件
    source_files = get_source_files(directory)
    if not source_files:
        print("No source files found.")
        sys.exit(0)

    # 检查每个文件的格式
    all_passed = True
    for file in source_files:
        if not check_clang_format(file):
            all_passed = False

    if all_passed:
        print("All files are properly formatted!")
        sys.exit(0)
    else:
        print("Some files need formatting. Please run clang-format.")
        sys.exit(1)

if __name__ == "__main__":
    main()