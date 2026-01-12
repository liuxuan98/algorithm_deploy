#!/usr/bin/env python3
"""
Code Formatter Script
Automatically format C/C++/CUDA files using clang-format
"""

import subprocess
import os
import sys
import argparse
import difflib
from pathlib import Path
from typing import List, Tuple


class CodeFormatter:
    def __init__(self, fix_mode: bool = False, show_diff: bool = True):
        """
        Initialize the code formatter

        Args:
            fix_mode: If True, automatically fix formatting issues
            show_diff: If True, show differences when formatting issues are found
        """
        self.fix_mode = fix_mode
        self.show_diff = show_diff

        # Get project root directory (parent of tools directory)
        self.script_dir = os.path.dirname(os.path.abspath(__file__))
        self.project_root = os.path.dirname(self.script_dir)

        self.source_dirs = [
            os.path.join(self.project_root, "kernel/"),
            os.path.join(self.project_root, "module"),
            os.path.join(self.project_root, "include/")
        ]
        self.file_extensions = [".h", ".cc", ".cu", ".hpp", ".cpp", ".c"]

        # Check if clang-format config exists in project root
        self.clang_format_config = os.path.join(self.project_root, '.clang-format')
        if not os.path.exists(self.clang_format_config):
            print("❌ Error: Missing .clang-format configuration file.")
            sys.exit(1)

        # Check if clang-format is available
        try:
            subprocess.run(["clang-format", "--version"],
                         stdout=subprocess.DEVNULL,
                         stderr=subprocess.DEVNULL,
                         check=True)
        except (subprocess.CalledProcessError, FileNotFoundError):
            print("❌ Error: clang-format not found. Please install clang-format.")
            sys.exit(1)

    def get_source_files(self) -> List[str]:
        """Get all source files to be formatted"""
        file_list = []

        for source_dir in self.source_dirs:
            if not os.path.exists(source_dir):
                print(f"⚠️  Warning: Directory {source_dir} not found, skipping...")
                continue

            for root, dirs, files in os.walk(source_dir, topdown=False):
                for name in files:
                    file_path = os.path.join(root, name)
                    if any(file_path.endswith(ext) for ext in self.file_extensions):
                        file_list.append(file_path)

        return file_list

    def format_file(self, file_path: str) -> bool:
        """
        Format a single file and check if changes are needed

        Returns:
            True if file needs formatting, False otherwise
        """
        try:
            # Generate formatted content using config file from project root
            result = subprocess.run(
                ["clang-format", f"-style=file:{self.clang_format_config}", file_path],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                check=True
            )
            formatted_content = result.stdout

            # Read original content
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                original_content = f.read()

            # Check if formatting is needed
            if original_content != formatted_content:
                if self.show_diff:
                    print(f"\n📄 File: {file_path}")
                    print("=" * 60)
                    diff = difflib.unified_diff(
                        original_content.splitlines(keepends=True),
                        formatted_content.splitlines(keepends=True),
                        fromfile=f"{file_path} (original)",
                        tofile=f"{file_path} (formatted)",
                        n=3
                    )
                    print(''.join(diff))

                if self.fix_mode:
                    with open(file_path, 'w', encoding='utf-8') as f:
                        f.write(formatted_content)
                    print(f"✅ Fixed: {file_path}")

                return True

            return False

        except subprocess.CalledProcessError as e:
            print(f"❌ Error formatting {file_path}: {e.stderr}")
            return False
        except Exception as e:
            print(f"❌ Error processing {file_path}: {str(e)}")
            return False

    def run(self) -> int:
        """Run the formatter on all source files"""
        print("🚀 Starting code formatting check...")
        print(f"📁 Scanning directories: {', '.join(self.source_dirs)}")
        print(f"📝 File extensions: {', '.join(self.file_extensions)}")
        print(f"🔧 Mode: {'Fix' if self.fix_mode else 'Check only'}")
        print()

        file_list = self.get_source_files()

        if not file_list:
            print("⚠️  No source files found to format.")
            return 0

        print(f"📊 Found {len(file_list)} files to check...")
        print()

        error_count = 0
        error_files = []

        for i, file_path in enumerate(file_list, 1):
            print(f"[{i}/{len(file_list)}] Checking {file_path}...", end=" ")

            if self.format_file(file_path):
                error_count += 1
                error_files.append(file_path)
                if not self.show_diff:
                    print("❌ Needs formatting")
            else:
                print("✅ OK")

        print()
        print("=" * 60)

        if error_count > 0:
            action = "Fixed" if self.fix_mode else "Found"
            print(f"📊 {action} {error_count} file(s) with formatting issues:")
            for file_path in error_files:
                print(f"  • {file_path}")

            if not self.fix_mode:
                print()
                print("💡 To automatically fix these issues, run:")
                print(f"   python {sys.argv[0]} --fix")

            return 1 if not self.fix_mode else 0
        else:
            print("🎉 All files are properly formatted!")
            return 0


def main():
    parser = argparse.ArgumentParser(
        description="Format C/C++/CUDA source code using clang-format",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python format_code.py              # Check formatting (default)
  python format_code.py --fix        # Fix formatting issues
  python format_code.py --no-diff    # Check without showing diffs
  python format_code.py --fix --no-diff  # Fix silently
        """
    )

    parser.add_argument(
        '--fix',
        action='store_true',
        help='Automatically fix formatting issues instead of just checking'
    )

    parser.add_argument(
        '--no-diff',
        action='store_true',
        help='Do not show diff output when formatting issues are found'
    )

    args = parser.parse_args()

    formatter = CodeFormatter(
        fix_mode=args.fix,
        show_diff=not args.no_diff
    )

    return formatter.run()


if __name__ == "__main__":
    sys.exit(main())
