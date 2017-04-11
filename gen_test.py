#!/usr/bin/python3
import sys, getopt, os, random

def usage():
    print("usage: " + sys.argv[0] + " -o output -m patlen -k k")

if len(sys.argv) != 7:
    usage()
    sys.exit()

opts, args = getopt.getopt(sys.argv[1:], "o:m:k:")
output = "test"
patlen = 10
k = 0

for op, value in opts:
    if op == "-o":
        output = value
    elif op == "-m":
        patlen = int(value);
    elif op == "-k":
        k = int(value)

#with open(os.path.join("src", output), "w") as outfile:
with open(output, "w") as outfile:
    docs = "docs"
    files = os.listdir(docs)
    for file in files:
        with open(os.path.join("docs",file),"r") as f:
            ind = []
            for i in range(10):
                ind.append(random.randint(0, 100))
            ind = set(ind)
            for index, line in enumerate(f.readlines()):
                if index in ind and len(line) >= patlen + 10:
                        outfile.write(line[9:9+patlen] + '\n')
                        outfile.write(str(k) + '\n')
    outfile.write("q\n")
