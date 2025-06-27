import subprocess
import os
import sys

# 配置需要检查的文件扩展名
FILE_EXTENSIONS = [".h",".hpp",".cpp", ".cc"]

# 查找所有符合条件的源文件
def get_source_files(directory):
    source_files = []
    for root, _, files in os.walk(directory):
        for file in files:
            if any(file.endswith(ext) for ext in FILE_EXTENSIONS):
                source_files.append(os.path.join(root, file))
    return source_files

# 运行 clang-tidy 静态检查
def run_clang_tidy(file, compile_commands_dir):
    try:
        # 使用 clang-tidy 检查
        subprocess.run(
            ["clang-tidy", file, "--quiet", f"--compile-commands-dir={compile_commands_dir}"],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        return True
    except subprocess.CalledProcessError as e:
        print(f"clang-tidy failed for {file}")
        return False

def main():
    # 默认从当前目录开始检查
    directory = "."
    if len(sys.argv) > 1:
        directory = sys.argv[1]

    # 查找 compile_commands.json
    compile_commands_dir = os.path.join(directory, "build")
    if not os.path.exists(os.path.join(compile_commands_dir, "compile_commands.json")):
        print("Error: compile_commands.json not found. Please generate it using CMake.")
        sys.exit(1)

    # 查找所有源文件
    source_files = get_source_files(directory)
    if not source_files:
        print("No source files found.")
        sys.exit(0)

    # 对每个文件运行 clang-tidy
    all_passed = True
    for file in source_files:
        if not run_clang_tidy(file, compile_commands_dir):
            all_passed = False

    if all_passed:
        print("clang-tidy check passed!")
        sys.exit(0)
    else:
        print("clang-tidy check failed for some files.")
        sys.exit(1)

if __name__ == "__main__":
    main()