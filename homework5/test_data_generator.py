import os
import sys
import random

def main():
    with open('input', 'w') as infile:
        infile.write(sys.argv[1] + '\n');
        for i in range(0, int(sys.argv[1])):
            a = random.randint(0, 255)
            b = random.randint(0, 255)
            c = random.randint(0, 255)
            infile.write('%d %d %d\n' % (a, b, c))

if __name__ == '__main__':
    main()
