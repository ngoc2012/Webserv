#!/usr/bin/env python3

import sys

print("Status: 200 OK\r\n")
print("Content-Type: plain/text; charset=utf-8\r\n\r\n")

for line in sys.stdin:
    # Process each line from stdin
    print("Read from stdin:", line.strip())
