#!/usr/bin/env python3
import subprocess
import os
import sys
import json

# .clang-tidy 配置文件必须存在
CLANG_TIDY_CFG = ".clang-tidy"
COMPILE_DB_DIR = "build_clang_tidy"  # compile_commands.json 所在目录
SOURCE_ROOT = "kernel"    # 只检查 kernel/ 下的文件

# 只检查特定后缀
ALLOWED_EXTS = (".h", ".cc", ".cpp", ".cxx", ".cuh", ".cu")

# 是否开启 -system-headers（不推荐，会进入 STL/三方头）
USE_SYSTEM_HEADERS = False

# 是否只输出问题，不因 warning 失败
# True: 只有 clang-tidy 崩溃才报错；False: 有 warning 也失败
TREAT_WARNINGS_AS_ERRORS = False


def main():
    # 检查 .clang-tidy
    if not os.path.exists(CLANG_TIDY_CFG):
        print(f"❌ Error: Missing {CLANG_TIDY_CFG}")
        sys.exit(1)

    # 检查 compile_commands.json
    compile_db_path = os.path.join(COMPILE_DB_DIR, "compile_commands.json")
    if not os.path.exists(compile_db_path):
        print(f"❌ Error: Missing compile_commands.json in '{COMPILE_DB_DIR}/'")
        print("💡 Please run: mkdir -p build && cd build && cmake ..")
        sys.exit(1)

    # 构建 run-clang-tidy 命令
    cmd = [
        "run-clang-tidy",
        "-p", COMPILE_DB_DIR,                    # 指向编译数据库
        "-header-filter", f"^{SOURCE_ROOT}/.*",  # 只显示 kernel/ 下的警告
    ]

    if not USE_SYSTEM_HEADERS:
        # 不显示系统头文件中的警告
        cmd.append("-quiet")

    # 可选：指定 checks
    # cmd += ["-checks", "modernize-*,readability-*"]

    print(f"🔍 Running: {' '.join(cmd)}")
    print(f"📌 Will check files under '{SOURCE_ROOT}/' with real build flags.")

    # 执行
    try:
        result = subprocess.run(cmd, capture_output=True, text=True)
    except FileNotFoundError:
        print("❌ Error: 'run-clang-tidy' not found.")
        print("💡 Install: sudo apt install clang-tools (or llvm)")
        sys.exit(1)

    # 输出结果
    if result.stdout:
        print(result.stdout, end="")

    if result.stderr:
        print("⚠️ Clang-Tidy stderr:", file=sys.stderr)
        print(result.stderr, file=sys.stderr)

    # 退出码处理
    if result.returncode != 0:
        if TREAT_WARNINGS_AS_ERRORS:
            print(f"❌ run-clang-tidy failed with return code {result.returncode}")
            sys.exit(1)
        else:
            print("✅ Warnings found, but not treated as errors.")
            sys.exit(0)
    else:
        print("✅ All good! No issues found.")
        sys.exit(0)

if __name__ == "__main__":
    main()
