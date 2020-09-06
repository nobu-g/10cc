#!/usr/bin/env python3

# Make assembly prettier
# usage:
# $ ./scripts/prasm.py tmp.s

import sys

def main():
    path = sys.argv[1]
    with open(path) as f:
        lines = f.readlines()

    with open(path, mode='wt') as f:
        depth = 0
        for line in lines:
            if line.startswith('.') or line == '\n':
                f.write(line)
                continue
            if line.rstrip().endswith(':'):
                depth = 1
                f.write(line)
                continue
            if line.lstrip().startswith('pop'):
                depth -= 1
            f.write('  ' * depth)
            if line.lstrip().startswith('push'):
                depth += 1
            f.write(line.lstrip())


if __name__ == '__main__':
    main()
