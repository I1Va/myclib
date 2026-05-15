#! /usr/bin/python3

import os
import sys

def scan_and_export(root_dirs, output_file="output.log", extensions=None, exclude_dirs=None, exclude_extensions=None):
    exclude_dirs = set(exclude_dirs) if exclude_dirs else set()
    exclude_extensions = set(exclude_extensions) if exclude_extensions else set()

    with open(output_file, 'w', encoding='utf-8') as out:
        for root_dir in root_dirs:
            root_dir = os.path.abspath(root_dir)
            
            for dirpath, dirnames, filenames in os.walk(root_dir, topdown=True):
                # Prune directories
                dirnames[:] = [d for d in dirnames if d not in exclude_dirs]

                for filename in filenames:
                    # 1. Filter by allowed extensions
                    if extensions and not any(filename.endswith(ext) for ext in extensions):
                        continue
                    
                    # 2. Filter by excluded extensions
                    if any(filename.endswith(ext) for ext in exclude_extensions) or filename in exclude_extensions:
                        continue
                        
                    full_path = os.path.join(dirpath, filename)
                    rel_path = os.path.relpath(full_path, root_dir)
                    
                    out.write(f"-{os.path.basename(root_dir)}/{rel_path}\n")
                    out.write("```\n")
                    try:
                        with open(full_path, 'r', encoding='utf-8', errors='ignore') as f:
                            out.write(f.read().rstrip() + "\n")
                    except OSError:
                        pass
                    out.write("```\n\n")

if __name__ == "__main__":
    # --- HARDCODED CONFIGURATION ---
    LANG_ONLY_MODE = False  # Set to False to include all allowed files
    # -------------------------------

    ignore_dirs = {".git", "__pycache__", "node_modules", ".venv", "build"}
    ignore_exts = {".bin", "makefile", ".exe", ".o", ".out", ".a"}

    if len(sys.argv) < 2:
        print("Usage: python utilite.py <folder1> [folder2 ...]")
        sys.exit(1)

    folders = sys.argv[1:]
    
    include_exts = None
    # Still allow standard extension filtering if '--' is used in args
    if "--" in folders:
        idx = folders.index("--")
        include_exts = folders[idx + 1:]
        folders = folders[:idx]

    # Hardcoded override: If LANG_ONLY_MODE is True, force .lang extension
    if LANG_ONLY_MODE:
        include_exts = [".lang"]

    scan_and_export(folders, extensions=include_exts, exclude_dirs=ignore_dirs, exclude_extensions=ignore_exts)
    
    status = "ENABLED" if LANG_ONLY_MODE else "DISABLED"
    print(f"Output saved to output.log. Lang-only mode: {status}")