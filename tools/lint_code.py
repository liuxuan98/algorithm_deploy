#!/usr/bin/env python3
"""
Code Linter Script
Automatically check and fix C/C++/CUDA code issues using clang-tidy
"""

import subprocess
import os
import sys
import argparse
import json
from pathlib import Path
from typing import List, Dict, Optional, Tuple
import tempfile
import shutil


class CodeLinter:
    def __init__(self, fix_mode: bool = False, show_output: bool = True,
                 config_file: str = ".clang-tidy", errors_only: bool = False):
        """
        Initialize the code linter

        Args:
            fix_mode: If True, automatically fix issues using clang-tidy --fix
            show_output: If True, show clang-tidy output
            config_file: Path to clang-tidy configuration file
            errors_only: If True, only show errors and not warnings
        """
        self.fix_mode = fix_mode
        self.show_output = show_output
        self.errors_only = errors_only

        # Get project root directory (parent of tools directory)
        self.script_dir = os.path.dirname(os.path.abspath(__file__))
        self.project_root = os.path.dirname(self.script_dir)

        # Set config file path relative to project root
        if config_file.startswith('.'):
            self.config_file = os.path.join(self.project_root, config_file)
        else:
            self.config_file = config_file

        self.source_dirs = [os.path.join(self.project_root, "kernel/")]  # Start with kernel only like original script
        self.file_extensions = [".h", ".cc", ".cu", ".hpp", ".cpp", ".c"]

        # Include paths for compilation (relative to project root)
        # Only include project-specific paths, third-party libraries are handled as system headers
        self.include_paths = [
            os.path.join(self.project_root, "kernel/"),
            os.path.join(self.project_root, "kernel/include"),
            os.path.join(self.project_root, "include"),
            self.project_root
        ]

        # Skip patterns (like mnn in original script)
        self.skip_patterns = [
            "mnn",  # Skip MNN files as per original script
            "third_party/",
            "build/",
            ".git/"
        ]

        # Check if clang-tidy config exists
        if not os.path.exists(self.config_file):
            print(f"⚠️  Warning: {self.config_file} configuration file not found.")
            print("Using default clang-tidy configuration.")

        # Check if clang-tidy is available
        try:
            subprocess.run(["clang-tidy", "--version"],
                         stdout=subprocess.DEVNULL,
                         stderr=subprocess.DEVNULL,
                         check=True)
        except (subprocess.CalledProcessError, FileNotFoundError):
            print("❌ Error: clang-tidy not found. Please install clang-tidy.")
            print("   Ubuntu/Debian: sudo apt-get install clang-tidy")
            sys.exit(1)

    def get_source_files(self) -> List[str]:
        """Get all source files to be linted"""
        file_list = []

        for source_dir in self.source_dirs:
            if not os.path.exists(source_dir):
                print(f"⚠️  Warning: Directory {source_dir} not found, skipping...")
                continue

            for root, dirs, files in os.walk(source_dir, topdown=False):
                for name in files:
                    file_path = os.path.join(root, name)

                    # Check file extension
                    if not any(file_path.endswith(ext) for ext in self.file_extensions):
                        continue

                    # Check skip patterns
                    if any(pattern in file_path for pattern in self.skip_patterns):
                        continue

                    # Check directory whitelist (like original script)
                    # Get relative path from project root to check directory name
                    rel_path = os.path.relpath(file_path, self.project_root)
                    dir_name = rel_path.split("/")[0] if "/" in rel_path else ""
                    if dir_name not in ["kernel", "module", "include", "test", "tools"]:
                        continue

                    file_list.append(file_path)

        return file_list

    def build_compile_command(self, file_path: str) -> List[str]:
        """Build compile command with include paths"""
        cmd = ["clang-tidy"]

        if self.fix_mode:
            cmd.append("--fix")

        # Add config file if exists
        if os.path.exists(self.config_file):
            cmd.extend(["--config-file", self.config_file])

        # Add header filter to only check project files, not third-party libraries
        # This will only check headers in kernel/, module/, include/, sample/, test/ directories
        # and exclude anything in third_party/ directories
        header_filter_pattern = r"^(?!.*third_party).*/(kernel|module|include|sample|test|tools)/.*\.(h|hpp|cc|cpp|c|cu)$"
        cmd.extend(["--header-filter", header_filter_pattern])

        cmd.append(file_path)
        cmd.append("--")

        # Add include paths - always use manual configuration for better control
        for include_path in self.include_paths:
            if os.path.exists(include_path):
                cmd.extend(["-I", include_path])

        # Add language specification
        cmd.extend(["-x", "c++"])

        # Note: C++ standard is specified in .clang-tidy config file

        # Add common compiler flags to reduce false positives
        cmd.extend([
            "-DNDEBUG",  # Define NDEBUG to avoid debug-only code issues
            "-D__STDC_CONSTANT_MACROS",
            "-D__STDC_LIMIT_MACROS",
            # Remove -fno-rtti as it causes typeid errors in cereal
            "-Wno-unknown-pragmas",  # Ignore unknown pragmas
            "-Wno-unused-parameter",  # Ignore unused parameters for now
            "-Wno-deprecated-declarations",  # Ignore deprecated warnings from third-party libs
            "-Wno-error",  # Don't treat warnings as errors for third-party code
        ])

        # Treat third-party includes as system headers to exclude them from analysis
        third_party_paths = [
            os.path.join(self.project_root, "third_party/cereal/include"),
            os.path.join(self.project_root, "third_party/rapidjson/include"),
            os.path.join(self.project_root, "third_party/opencv/include"),
            os.path.join(self.project_root, "third_party/openvino/include"),
            os.path.join(self.project_root, "third_party/mnn/Linux/x86_64/include"),
            os.path.join(self.project_root, "third_party/RSLog/Linux/include"),
        ]
        
        for tp_path in third_party_paths:
            if os.path.exists(tp_path):
                cmd.extend(["-isystem", tp_path])

        # Platform-specific defines
        import platform
        if platform.system() == "Linux":
            cmd.extend(["-D__linux__", "-DLINUX"])

        return cmd

    def lint_file(self, file_path: str) -> Tuple[bool, str]:
        """
        Lint a single file and return results

        Returns:
            Tuple of (has_issues, output_text)
        """
        try:
            cmd = self.build_compile_command(file_path)

            result = subprocess.run(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                timeout=30  # 30 second timeout per file
            )

            output = result.stdout
            has_issues = result.returncode != 0

            # Filter out some common noise
            if output:
                lines = output.split('\n')
                filtered_lines = []
                for line in lines:
                    # Skip empty lines and common noise
                    if (line.strip() and
                        not line.startswith('Skipping') and
                        not line.startswith('Unable to handle') and
                        'compilation database' not in line.lower()):
                        
                        # Skip any line that mentions third_party paths
                        if 'third_party' in line:
                            continue
                        
                        # If errors_only is True, filter out warning lines
                        if self.errors_only:
                            # Only keep error lines and related context
                            if ('error:' in line.lower() or 
                                line.strip().startswith('^') or  # Error indicator lines
                                line.strip().startswith('~') or  # Error indicator lines
                                (line.strip() and not 'warning:' in line.lower() and 
                                 not 'note:' in line.lower() and
                                 not line.strip().endswith('warnings generated.'))):
                                filtered_lines.append(line)
                        else:
                            filtered_lines.append(line)
                
                output = '\n'.join(filtered_lines)

            # If errors_only is True, only consider it as having issues if there are actual errors
            # Also check if the remaining output contains any real errors (not just third_party issues)
            if self.errors_only:
                has_issues = 'error:' in output.lower() and 'third_party' not in output.lower()
            else:
                # For normal mode, also ignore third_party related issues
                if 'error:' in output.lower() and 'third_party' in output.lower():
                    # Check if there are non-third_party errors
                    error_lines = [line for line in output.split('\n') if 'error:' in line.lower()]
                    non_third_party_errors = [line for line in error_lines if 'third_party' not in line]
                    has_issues = len(non_third_party_errors) > 0

            return has_issues, output

        except subprocess.TimeoutExpired:
            return True, f"Timeout: {file_path} took too long to process"
        except subprocess.CalledProcessError as e:
            return True, f"Error processing {file_path}: {e}"
        except Exception as e:
            return True, f"Unexpected error processing {file_path}: {str(e)}"

    def create_compilation_database(self) -> Optional[str]:
        """Create a simple compilation database for better clang-tidy results"""
        try:
            # Check if compile_commands.json already exists
            compile_db_path = os.path.join(self.project_root, "compile_commands.json")
            if os.path.exists(compile_db_path):
                print("📁 Using existing compile_commands.json")
                return compile_db_path

            # Try to generate one using cmake if CMakeLists.txt exists
            cmake_file = os.path.join(self.project_root, "CMakeLists.txt")
            if os.path.exists(cmake_file):
                print("🔨 Generating compilation database with CMake...")
                build_dir = tempfile.mkdtemp(prefix="clang_tidy_build_")
                try:
                    # Change to project root directory
                    original_cwd = os.getcwd()
                    os.chdir(self.project_root)
                    
                    # Run cmake to generate compile commands
                    result = subprocess.run([
                        "cmake", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
                        "-B", build_dir, "-S", "."
                    ], check=True, capture_output=True, text=True)
                    
                    compile_db = os.path.join(build_dir, "compile_commands.json")
                    if os.path.exists(compile_db):
                        shutil.copy(compile_db, "compile_commands.json")
                        print("✅ Generated compile_commands.json")
                        return "compile_commands.json"
                    else:
                        print("⚠️  Failed to generate compile_commands.json")
                        
                except subprocess.CalledProcessError as e:
                    print(f"⚠️  CMake failed: {e.stderr}")
                except Exception as e:
                    print(f"⚠️  Error generating compilation database: {e}")
                finally:
                    os.chdir(original_cwd)
                    shutil.rmtree(build_dir, ignore_errors=True)

            return None

        except Exception as e:
            print(f"⚠️  Error in create_compilation_database: {e}")
            return None

    def run(self) -> int:
        """Run the linter on all source files"""
        print("🔍 Starting code linting check...")
        print(f"📁 Scanning directories: {', '.join(self.source_dirs)}")
        print(f"📝 File extensions: {', '.join(self.file_extensions)}")
        print(f"🔧 Mode: {'Fix' if self.fix_mode else 'Check only'}")
        if self.errors_only:
            print("⚠️  Filter: Errors only (warnings hidden)")
        print("🚫 Third-party libraries: Excluded from checking")
        print(f"⚙️  Config: {self.config_file}")
        print(f"📋 Include paths: {len([p for p in self.include_paths if os.path.exists(p)])} valid paths")
        print()

        # Validate that critical headers can be found
        self.validate_include_paths()

        file_list = self.get_source_files()

        if not file_list:
            print("⚠️  No source files found to lint.")
            return 0

        print(f"📊 Found {len(file_list)} files to check...")
        print()

        error_count = 0
        error_files = []
        warning_count = 0

        for i, file_path in enumerate(file_list, 1):
            print(f"[{i}/{len(file_list)}] Checking {file_path}...", end=" ")

            has_issues, output = self.lint_file(file_path)

            if has_issues:
                if 'error:' in output.lower():
                    error_count += 1
                    error_files.append(file_path)
                    print("❌ Errors")
                elif not self.errors_only:  # Only count warnings if not in errors_only mode
                    warning_count += 1
                    print("⚠️  Warnings")
                else:
                    # In errors_only mode, if no errors but has_issues was True, treat as OK
                    print("✅ OK")

                if self.show_output and output.strip():
                    print(f"\n📄 File: {file_path}")
                    print("=" * 60)
                    print(output)
                    print("=" * 60)

                if self.fix_mode and 'Fixed' in output:
                    print(f"✅ Fixed issues in: {file_path}")
            else:
                print("✅ OK")

        print()
        print("=" * 60)

        # Summary
        if error_count > 0 or warning_count > 0:
            print(f"📊 Linting Summary:")
            if error_count > 0:
                print(f"   ❌ {error_count} file(s) with errors")
            if warning_count > 0 and not self.errors_only:
                print(f"   ⚠️  {warning_count} file(s) with warnings")

            if error_files:
                print(f"\n📋 Files with errors:")
                for file_path in error_files:
                    print(f"  • {file_path}")

            if not self.fix_mode:
                print()
                print("💡 To automatically fix issues, run:")
                print(f"   python3 {sys.argv[0]} --fix")

            return 1 if error_count > 0 else 0
        else:
            if self.errors_only:
                print("🎉 No errors found in checked files!")
            else:
                print("🎉 All files passed linting checks!")
            return 0

    def check_and_fix_config(self):
        """Check and potentially fix common configuration issues"""
        print("🔧 Checking clang-tidy configuration...")

        # Check if include paths exist
        missing_paths = []
        for path in self.include_paths:
            if not os.path.exists(path):
                missing_paths.append(path)

        if missing_paths:
            print(f"⚠️  Missing include paths: {missing_paths}")
            # Filter out missing paths
            self.include_paths = [p for p in self.include_paths if os.path.exists(p)]
            print(f"✅ Using available paths: {self.include_paths}")

    def validate_include_paths(self):
        """Validate that critical third-party headers can be found"""
        print("🔍 Checking third-party library headers (as system headers)...")
        
        third_party_headers = [
            ("cereal/cereal.hpp", "third_party/cereal/include"),
            ("rapidjson/document.h", "third_party/rapidjson/include"),
            ("RSLog/RSLog.h", "third_party/RSLog/Linux/include")
        ]
        
        missing_headers = []
        found_headers = []
        
        for header, relative_path in third_party_headers:
            full_path = os.path.join(self.project_root, relative_path, header)
            if os.path.exists(full_path):
                found_headers.append(header)
            else:
                missing_headers.append((header, full_path))
        
        if found_headers:
            print(f"✅ Found system headers: {', '.join(found_headers)}")
            
        if missing_headers:
            print("⚠️  Missing system headers:")
            for header, full_path in missing_headers:
                print(f"   • {header} (expected at {full_path})")
        
        print()


def main():
    parser = argparse.ArgumentParser(
        description="Lint C/C++/CUDA source code using clang-tidy",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 lint_code.py                    # Check code (default)
  python3 lint_code.py --fix              # Fix issues automatically
  python3 lint_code.py --no-output        # Check without detailed output
  python3 lint_code.py --errors-only      # Only show errors, hide warnings
  python3 lint_code.py --all-dirs         # Check all directories (kernel, module, include)
  python3 lint_code.py --fix --no-output  # Fix silently
  python3 lint_code.py --config .clang-tidy  # Use specific config file
  python3 lint_code.py --errors-only --fix   # Fix errors only
        """
    )

    parser.add_argument(
        '--fix',
        action='store_true',
        help='Automatically fix issues instead of just checking'
    )

    parser.add_argument(
        '--no-output',
        action='store_true',
        help='Do not show detailed clang-tidy output'
    )

    parser.add_argument(
        '--config',
        default='.clang-tidy',
        help='Path to clang-tidy configuration file (default: .clang-tidy)'
    )

    parser.add_argument(
        '--errors-only',
        action='store_true',
        help='Only show errors, hide warnings and notes'
    )

    parser.add_argument(
        '--all-dirs',
        action='store_true',
        help='Check all directories (kernel, module, include) instead of just kernel'
    )

    args = parser.parse_args()

    linter = CodeLinter(
        fix_mode=args.fix,
        show_output=not args.no_output,
        config_file=args.config,
        errors_only=args.errors_only
    )

    # Expand source directories if requested
    if args.all_dirs:
        linter.source_dirs = [
            os.path.join(linter.project_root, "kernel/"),
            os.path.join(linter.project_root, "module/"),
            os.path.join(linter.project_root, "include/")
        ]

    linter.check_and_fix_config()

    return linter.run()


if __name__ == "__main__":
    sys.exit(main())
