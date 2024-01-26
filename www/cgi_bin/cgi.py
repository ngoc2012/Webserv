#!/usr/bin/env python3

import sys
import subprocess

# Check if the correct number of arguments is provided
if len(sys.argv) != 2:
    print("Usage: ./execute.py <filename.py>")
    sys.exit(1)

# Get the filename from the command-line arguments
filename = sys.argv[1]

# Check if the file exists
if not subprocess.run(["test", "-f", filename]).returncode == 0:
    print(f"Error: File '{filename}' not found.")
    sys.exit(1)

# Check if the file has a .py extension
if not filename.endswith(".py"):
    print("Error: Only Python files are allowed.")
    sys.exit(1)

# Use subprocess.run to execute the Python file
try:
    result = subprocess.run(["python3", filename], check=True, capture_output=True, text=True)
    print(result.stdout, end="")
except subprocess.CalledProcessError as e:
    print(f"Error: Failed to execute '{filename}'.")
    print("Error message:")
    print(e.stderr, end="")
    sys.exit(1)

