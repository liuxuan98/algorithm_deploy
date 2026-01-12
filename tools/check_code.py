#!/usr/bin/env python3
"""
Comprehensive Code Checker
Runs both clang-format and clang-tidy checks on C/C++/CUDA code
"""

import subprocess
import sys
import argparse
import os
from typing import Tuple


def run_format_check(fix_mode: bool = False, show_diff: bool = True) -> Tuple[int, str]:
    """Run clang-format check"""
    print("🎨 Running code formatting check...")
    print("=" * 60)

    script_dir = os.path.dirname(os.path.abspath(__file__))
    format_script = os.path.join(script_dir, "format_code.py")
    cmd = ["python3", format_script]
    if fix_mode:
        cmd.append("--fix")
    if not show_diff:
        cmd.append("--no-diff")

    try:
        result = subprocess.run(cmd, capture_output=True, text=True)
        return result.returncode, result.stdout + result.stderr
    except FileNotFoundError:
        return 1, "❌ format_code.py not found"


def run_lint_check(fix_mode: bool = False, show_output: bool = True,
                   all_dirs: bool = False) -> Tuple[int, str]:
    """Run clang-tidy check"""
    print("🔍 Running code linting check...")
    print("=" * 60)

    script_dir = os.path.dirname(os.path.abspath(__file__))
    lint_script = os.path.join(script_dir, "lint_code.py")
    cmd = ["python3", lint_script]
    if fix_mode:
        cmd.append("--fix")
    if not show_output:
        cmd.append("--no-output")
    if all_dirs:
        cmd.append("--all-dirs")

    try:
        result = subprocess.run(cmd, capture_output=True, text=True)
        return result.returncode, result.stdout + result.stderr
    except FileNotFoundError:
        return 1, "❌ lint_code.py not found"


def main():
    parser = argparse.ArgumentParser(
        description="Comprehensive code quality checker (format + lint)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 check_code.py                           # Check both format and lint
  python3 check_code.py --fix                     # Fix both format and lint issues
  python3 check_code.py --format-only             # Only check formatting
  python3 check_code.py --lint-only               # Only check linting
  python3 check_code.py --fix --format-only       # Only fix formatting
  python3 check_code.py --quiet                   # Minimal output
        """
    )

    parser.add_argument(
        '--fix',
        action='store_true',
        help='Automatically fix issues instead of just checking'
    )

    parser.add_argument(
        '--format-only',
        action='store_true',
        help='Only run formatting checks'
    )

    parser.add_argument(
        '--lint-only',
        action='store_true',
        help='Only run linting checks'
    )

    parser.add_argument(
        '--quiet',
        action='store_true',
        help='Minimal output (no detailed diffs/lint output)'
    )

    parser.add_argument(
        '--all-dirs',
        action='store_true',
        help='Check all directories (kernel, module, include) instead of just kernel'
    )

    args = parser.parse_args()

    if args.format_only and args.lint_only:
        print("❌ Error: Cannot specify both --format-only and --lint-only")
        return 1

    print("🚀 Starting comprehensive code quality check...")
    print(f"🔧 Mode: {'Fix' if args.fix else 'Check only'}")
    if args.all_dirs:
        print("📁 Scope: All directories (kernel, module, include)")
    else:
        print("📁 Scope: Kernel directory only")
    print()

    total_errors = 0

    # Run formatting check
    if not args.lint_only:
        format_code, format_output = run_format_check(
            fix_mode=args.fix,
            show_diff=not args.quiet
        )

        if not args.quiet:
            print(format_output)

        if format_code != 0:
            total_errors += 1
            if args.quiet:
                print("❌ Formatting issues found")
        elif args.quiet:
            print("✅ Formatting OK")

        print()

    # Run linting check
    if not args.format_only:
        lint_code, lint_output = run_lint_check(
            fix_mode=args.fix,
            show_output=not args.quiet,
            all_dirs=args.all_dirs
        )

        if not args.quiet:
            print(lint_output)

        if lint_code != 0:
            total_errors += 1
            if args.quiet:
                print("❌ Linting issues found")
        elif args.quiet:
            print("✅ Linting OK")

        print()

    # Final summary
    print("=" * 60)
    if total_errors == 0:
        print("🎉 All checks passed! Code quality looks good.")
        return 0
    else:
        action = "fixed" if args.fix else "found"
        print(f"📊 Code quality issues {action}:")

        if not args.lint_only and format_code != 0:
            print("  • Formatting issues")
        if not args.format_only and lint_code != 0:
            print("  • Linting issues")

        if not args.fix:
            print()
            print("💡 To automatically fix issues, run:")
            print(f"   python3 {sys.argv[0]} --fix")

        return 1 if not args.fix else 0


if __name__ == "__main__":
    sys.exit(main())
