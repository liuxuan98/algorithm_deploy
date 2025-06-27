#根据参数调用不同平台的构建编译脚本
#调用格式 python Build.py Windows/Linux/Mac Release/Debug Publish/CI 不支持debug模式的编译

import os
import sys

# 获取当前脚本所在目录
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# 定义支持的平台和构建类型
SUPPORTED_PLATFORMS = ["windows", "linux", "mac"]
SUPPORTED_BUILD_TYPES = ["release", "debug"]
SUPPORTED_BUILD_MODES = ["publish", "ci"]

def print_usage():
    print("Usage: python build.py <platform> <build_type> <build_mode>")
    print("Example: python build.py Windows Release Publish")
    print("\n<platform>:       Windows / Linux / Mac")
    print("<build_type>:     Release / Debug")
    print("<build_mode>:     Publish / CI")
    sys.exit(1)

def validate_args(platform, build_type, build_mode):
    if platform.lower() not in SUPPORTED_PLATFORMS:
        print(f"[ERROR] Unsupported platform: {platform}")
        print_usage()

    if build_type.lower() not in SUPPORTED_BUILD_TYPES:
        print(f"[ERROR] Unsupported build type: {build_type}")
        print_usage()

    if build_mode.lower() not in SUPPORTED_BUILD_MODES:
        print(f"[ERROR] Unsupported build mode: {build_mode}")
        print_usage()

def run_script(script_path, args=None):
    if not os.path.exists(script_path):
        print(f"[ERROR] Script not found: {script_path}")
        sys.exit(1)

    cmd = script_path
    print("[INFO] Running script: %s", script_path)
    print("[INFO] Running command: %s", args)
    if args:
        cmd += " " + " ".join(args)

    print(f"Running command: {cmd}")
    ret_code = os.system(cmd)
    if ret_code != 0:
        print(f"[ERROR] Build failed with code {ret_code}")
        sys.exit(ret_code)
    else:
        print("Build Done.")

def main():
    if len(sys.argv) < 4:
        print_usage()

    platform = sys.argv[1]
    build_type = sys.argv[2]
    build_mode = sys.argv[3]

    # 参数校验
    validate_args(platform, build_type, build_mode)

    # 构建脚本路径映射（根据您的项目结构调整）
    script_map = {
        "windows": {
            "ci": os.path.join(SCRIPT_DIR, "windows", "build_win_ci.bat"),
            "publish": os.path.join(SCRIPT_DIR, "windows", "build_win_publish.bat")
        },
        "linux": {
            "ci": os.path.join(SCRIPT_DIR, "linux", "build_linux_ci.sh"),
            "publish": os.path.join(SCRIPT_DIR, "linux", "build_linux_publish.sh")
        },
        "mac": {
            "ci": os.path.join(SCRIPT_DIR, "mac", "build_mac_ci.sh"),
            "publish": os.path.join(SCRIPT_DIR, "mac", "build_mac_publish.sh")
        }
    }

    target_script = script_map[platform.lower()][build_mode.lower()]

    # 调用对应的脚本，并传递参数
    run_script(target_script, [build_type])

if __name__ == "__main__":
    main()



