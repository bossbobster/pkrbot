#!/bin/bash

# Get the directory where this script is located (works on macOS)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

PWD="$(pwd)"

# Change to the script directory
cd "$SCRIPT_DIR"

# remove all build files (including generated c files)
rm -rf build *.so *.c

# remove all __pycache__ files
rm -rf __pycache__

# build the extension using the Python in the .venv
"${VIRTUAL_ENV}/bin/python" setup.py build_ext --inplace

# return to the original directory
cd "$PWD"