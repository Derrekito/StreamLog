#!/usr/bin/env python3
"""
liblog Distribution Package Creator

Creates a distribution-ready copy of the liblog project with:
- All comments stripped from source files (optional)
- Distribution statement added to all text-based files
- Only essential files for building and running included
- Manual PDF placed at the distribution root
- No markdown files except README.md

Usage:
    ./scripts/make_dist.py <output_directory>
    ./scripts/make_dist.py /path/to/dist
    ./scripts/make_dist.py /media/usb/liblog --no-strip
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
from datetime import date
from pathlib import Path
from typing import Callable, Optional


DISTRIBUTION_STATEMENT = """Distribution Statement C. Distribution authorized to U.S. Government agencies and their contractors CTI {date}. Other requests for this document must be referred to AFRL RV."""


def get_distribution_comment(filepath: Path) -> Optional[str]:
    """Generate distribution statement as a comment block for the given file type."""
    name = filepath.name.lower()
    suffix = filepath.suffix.lower()

    statement = DISTRIBUTION_STATEMENT.format(date=date.today().strftime("%Y-%m-%d"))

    # C-style block comment
    if suffix in ('.c', '.cpp', '.cc', '.cxx', '.h', '.hpp', '.hxx',
                   '.cu', '.cuh', '.hip'):
        return f"/*\n * {statement}\n */\n\n"

    # Hash-style comments
    if suffix in ('.sh', '.bash', '.py', '.mk') or name == 'makefile':
        lines = []
        lines.append("#")
        lines.append(f"# {statement}")
        lines.append("#")
        lines.append("")
        return '\n'.join(lines) + '\n'

    # Markdown (HTML-style comment)
    if suffix == '.md':
        return f"<!--\n{statement}\n-->\n\n"

    # Default for other text files - use hash comments
    return f"#\n# {statement}\n#\n\n"


SCRIPT_DIR = Path(__file__).parent.resolve()
PROJECT_ROOT = SCRIPT_DIR.parent.resolve()

MANUAL_DIR = PROJECT_ROOT / "manual"
MANUAL_PDF = MANUAL_DIR / "output" / "liblog-manual.pdf"


def build_manual() -> bool:
    """Build the PDF manual using make."""
    if not MANUAL_DIR.exists():
        print(f"Warning: Manual directory not found at {MANUAL_DIR}")
        return False

    print("Building PDF manual (force rebuild)...")
    try:
        result = subprocess.run(
            ["make", "-B", "pdf"],
            cwd=MANUAL_DIR,
            capture_output=True,
            text=True
        )
        if result.returncode != 0:
            print(f"  Warning: Manual build failed:")
            print(f"  {result.stderr}")
            return False
        print("  Manual built successfully.")
        return True
    except FileNotFoundError:
        print("  Warning: 'make' command not found. Skipping manual build.")
        return False
    except Exception as e:
        print(f"  Warning: Could not build manual: {e}")
        return False


EXCLUDE_DIRS = {
    ".git",
    ".claude",
    "__pycache__",
    ".pytest_cache",
    "node_modules",
    "venv",
    "manual",
    "build",
    "docs",
    ".mypy_cache",
}

EXCLUDE_FILES = {
    ".gitignore",
    "*.o",
    "*.d",
    "*.a",
    "*.so",
    "*.dylib",
    "*.pyc",
    "*.pyo",
    ".DS_Store",
    "Thumbs.db",
    "*.log",
    "*.swp",
    "*.swo",
    "*~",
    "package.json",
    "package-lock.json",
    "latexmkrc",
}

INCLUDE_DIRS = {
    "src",
    "mk",
    "examples",
    "scripts",
}

INCLUDE_ROOT_FILES = {
    "Makefile",
}


def strip_c_comments(content: str) -> str:
    result = []
    i = 0
    in_string = False
    string_char = None
    in_single_comment = False
    in_multi_comment = False

    while i < len(content):
        if in_single_comment:
            if content[i] == '\n':
                in_single_comment = False
                result.append('\n')
            i += 1
            continue

        if in_multi_comment:
            if i + 1 < len(content) and content[i:i+2] == '*/':
                in_multi_comment = False
                i += 2
            else:
                if content[i] == '\n':
                    result.append('\n')
                i += 1
            continue

        if in_string:
            result.append(content[i])
            if content[i] == string_char and (i == 0 or content[i-1] != '\\'):
                in_string = False
                string_char = None
            i += 1
            continue

        if content[i] in ('"', "'"):
            in_string = True
            string_char = content[i]
            result.append(content[i])
            i += 1
            continue

        if i + 1 < len(content) and content[i:i+2] == '//':
            in_single_comment = True
            i += 2
            continue

        if i + 1 < len(content) and content[i:i+2] == '/*':
            in_multi_comment = True
            i += 2
            continue

        result.append(content[i])
        i += 1

    text = ''.join(result)

    lines = text.split('\n')
    cleaned = []
    prev_empty = False

    for line in lines:
        stripped = line.rstrip()
        is_empty = len(stripped) == 0

        if is_empty and prev_empty:
            continue

        cleaned.append(stripped)
        prev_empty = is_empty

    while cleaned and not cleaned[-1]:
        cleaned.pop()
    while cleaned and not cleaned[0]:
        cleaned.pop(0)

    return '\n'.join(cleaned) + '\n' if cleaned else ''


def strip_shell_comments(content: str) -> str:
    lines = content.split('\n')
    result = []

    for line in lines:
        stripped = line.strip()

        if stripped.startswith('#!'):
            result.append(line)
            continue

        if stripped.startswith('#'):
            continue

        if '#' in line:
            in_string = False
            string_char = None
            new_line = []
            j = 0
            while j < len(line):
                char = line[j]
                if not in_string and char in ('"', "'"):
                    in_string = True
                    string_char = char
                    new_line.append(char)
                elif in_string and char == string_char and (j == 0 or line[j-1] != '\\'):
                    in_string = False
                    string_char = None
                    new_line.append(char)
                elif not in_string and char == '#':
                    break
                else:
                    new_line.append(char)
                j += 1
            line = ''.join(new_line).rstrip()

        if line.strip() or not result or result[-1].strip():
            result.append(line)

    while result and not result[-1].strip():
        result.pop()

    return '\n'.join(result) + '\n' if result else ''


def strip_makefile_comments(content: str) -> str:
    lines = content.split('\n')
    result = []

    for line in lines:
        stripped = line.strip()

        if stripped.startswith('##@') or stripped.startswith('##'):
            result.append(line)
            continue

        if stripped.startswith('#'):
            continue

        if '##' in line:
            result.append(line)
            continue

        if '#' in line:
            in_string = False
            string_char = None
            new_line = []
            j = 0
            while j < len(line):
                char = line[j]
                if not in_string and char in ('"', "'"):
                    in_string = True
                    string_char = char
                    new_line.append(char)
                elif in_string and char == string_char and (j == 0 or line[j-1] != '\\'):
                    in_string = False
                    string_char = None
                    new_line.append(char)
                elif not in_string and char == '#':
                    break
                else:
                    new_line.append(char)
                j += 1
            line = ''.join(new_line).rstrip()

        if line.strip() or not result or result[-1].strip():
            result.append(line)

    while result and not result[-1].strip():
        result.pop()

    return '\n'.join(result) + '\n' if result else ''


def get_stripper(filepath: Path) -> Optional[Callable[[str], str]]:
    name = filepath.name.lower()
    suffix = filepath.suffix.lower()

    if suffix in ('.c', '.cpp', '.cc', '.cxx', '.h', '.hpp', '.hxx',
                   '.cu', '.cuh', '.hip'):
        return strip_c_comments
    elif suffix in ('.sh', '.bash'):
        return strip_shell_comments
    elif suffix == '.mk' or name == 'makefile':
        return strip_makefile_comments

    return None


def should_exclude_dir(dirname: str) -> bool:
    for pattern in EXCLUDE_DIRS:
        if pattern.startswith('*'):
            if dirname.endswith(pattern[1:]):
                return True
        elif dirname == pattern:
            return True
    return False


def should_exclude_file(filename: str) -> bool:
    for pattern in EXCLUDE_FILES:
        if pattern.startswith('*'):
            if filename.endswith(pattern[1:]):
                return True
        elif filename == pattern:
            return True
    return False


def is_binary_file(filepath: Path) -> bool:
    try:
        with open(filepath, 'rb') as f:
            chunk = f.read(8192)
            if b'\x00' in chunk:
                return True
            return False
    except IOError:
        return True


def add_distribution_statement(content: str, filepath: Path) -> str:
    """Add distribution statement to the beginning of file content.

    For files with shebangs, the statement is inserted after the shebang line.
    """
    dist_comment = get_distribution_comment(filepath)
    if not dist_comment:
        return content

    # Check for shebang
    if content.startswith('#!'):
        # Find end of first line
        newline_idx = content.find('\n')
        if newline_idx != -1:
            shebang = content[:newline_idx + 1]
            rest = content[newline_idx + 1:]
            return shebang + dist_comment + rest
        else:
            return content + '\n' + dist_comment

    return dist_comment + content


def copy_and_process_file(src: Path, dst: Path, strip_comments: bool = True) -> bool:
    dst.parent.mkdir(parents=True, exist_ok=True)

    if is_binary_file(src):
        shutil.copy2(src, dst)
        return True

    stripper = get_stripper(src) if strip_comments else None

    try:
        with open(src, 'r', encoding='utf-8', errors='replace') as f:
            content = f.read()

        # Strip comments if applicable
        if stripper:
            content = stripper(content)

        # Add distribution statement to all text files
        content = add_distribution_statement(content, src)

        with open(dst, 'w', encoding='utf-8') as f:
            f.write(content)

        shutil.copystat(src, dst)
        return True
    except Exception as e:
        print(f"  Warning: Could not process {src}: {e}", file=sys.stderr)
        shutil.copy2(src, dst)
        return True


def process_directory(src_dir: Path, dst_dir: Path, rel_path: Path = Path('.'),
                      strip_comments: bool = True) -> int:
    count = 0

    try:
        entries = list(src_dir.iterdir())
    except PermissionError:
        print(f"  Warning: Cannot access {src_dir}", file=sys.stderr)
        return 0

    for entry in sorted(entries):
        if entry.is_dir():
            if should_exclude_dir(entry.name):
                continue

            count += process_directory(
                entry,
                dst_dir / entry.name,
                rel_path / entry.name,
                strip_comments=strip_comments
            )
        else:
            if should_exclude_file(entry.name):
                continue

            if entry.suffix.lower() == '.md':
                continue

            if copy_and_process_file(entry, dst_dir / entry.name, strip_comments=strip_comments):
                count += 1
                print(f"  {rel_path / entry.name}")

    return count


def create_distribution(output_dir: Path, strip_comments: bool = True) -> bool:
    if output_dir.exists():
        print(f"Error: Output directory already exists: {output_dir}")
        print("Please remove it first or choose a different location.")
        return False

    mode = "with comments stripped" if strip_comments else "with comments preserved"
    print(f"Creating distribution in: {output_dir}")
    print(f"Mode: {mode}")
    print()

    output_dir.mkdir(parents=True)

    total_files = 0

    # Build the manual first to ensure latest content
    build_manual()

    if MANUAL_PDF.exists():
        print("Copying manual PDF to distribution root...")
        dst_pdf = output_dir / MANUAL_PDF.name
        shutil.copy2(MANUAL_PDF, dst_pdf)
        print(f"  {MANUAL_PDF.name}")
        total_files += 1
    else:
        print(f"Warning: Manual PDF not found at {MANUAL_PDF}")
        print("  You may need to build it first with: cd manual && make pdf")

    print()
    print("Processing root files...")
    for filename in sorted(INCLUDE_ROOT_FILES):
        src = PROJECT_ROOT / filename
        if src.exists():
            if copy_and_process_file(src, output_dir / filename, strip_comments=strip_comments):
                total_files += 1
                print(f"  {filename}")

    print()
    print("Processing source directories...")
    for dirname in sorted(INCLUDE_DIRS):
        src_dir = PROJECT_ROOT / dirname
        if src_dir.exists() and src_dir.is_dir():
            print(f"\n  [{dirname}/]")
            count = process_directory(src_dir, output_dir / dirname, Path(dirname),
                                       strip_comments=strip_comments)
            total_files += count

    print()
    print("=" * 60)
    print(f"Distribution created successfully!")
    print(f"  Location: {output_dir}")
    print(f"  Total files: {total_files}")
    print(f"  Comments: {'stripped' if strip_comments else 'preserved'}")
    print(f"  Distribution Statement: Added to all text files")

    if MANUAL_PDF.exists():
        print(f"  Manual: {MANUAL_PDF.name} (at root)")

    total_size = sum(
        f.stat().st_size
        for f in output_dir.rglob('*')
        if f.is_file()
    )
    print(f"  Total size: {total_size / 1024:.2f} KB")
    print("=" * 60)

    return True


def main():
    parser = argparse.ArgumentParser(
        description='Create a distribution-ready copy of liblog',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s ./dist                    Create distribution in ./dist (comments stripped)
  %(prog)s ./dist --no-strip         Create distribution with comments preserved
  %(prog)s /media/usb/liblog         Create distribution on USB drive
  %(prog)s ~/Desktop/liblog-dist     Create distribution on desktop
  %(prog)s ./dist -n                 Dry run - show what would be copied

The distribution will contain:
  - liblog-manual.pdf at the root
  - All source files (comments stripped by default, use --no-strip to preserve)
  - Distribution Statement C added to all text-based files
  - No markdown files, build artifacts, or development files
"""
    )

    parser.add_argument(
        'output_dir',
        type=Path,
        help='Directory to create the distribution in (must not exist)'
    )

    parser.add_argument(
        '--dry-run', '-n',
        action='store_true',
        help='Show what would be copied without copying'
    )

    parser.add_argument(
        '--no-strip', '--keep-comments',
        dest='keep_comments',
        action='store_true',
        help='Preserve comments in source files (default: strip comments)'
    )

    parser.add_argument(
        '--strip',
        dest='strip_comments',
        action='store_true',
        default=True,
        help='Strip comments from source files (default behavior)'
    )

    args = parser.parse_args()

    strip_comments = not args.keep_comments

    if not PROJECT_ROOT.exists():
        print(f"Error: Project root not found: {PROJECT_ROOT}")
        sys.exit(1)

    output_dir = args.output_dir.resolve()

    if args.dry_run:
        mode = "with comments stripped" if strip_comments else "with comments preserved"
        print("DRY RUN - showing what would be copied:")
        print(f"Output directory: {output_dir}")
        print(f"Mode: {mode}")
        print()
        if MANUAL_PDF.exists():
            print(f"  {MANUAL_PDF.name} -> {output_dir / MANUAL_PDF.name}")
        for filename in sorted(INCLUDE_ROOT_FILES):
            src = PROJECT_ROOT / filename
            if src.exists():
                print(f"  {filename}")
        for dirname in sorted(INCLUDE_DIRS):
            src_dir = PROJECT_ROOT / dirname
            if src_dir.exists():
                print(f"  {dirname}/ (directory)")
        sys.exit(0)

    success = create_distribution(output_dir, strip_comments=strip_comments)
    sys.exit(0 if success else 1)


if __name__ == '__main__':
    main()
