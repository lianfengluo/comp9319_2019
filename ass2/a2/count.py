from collections import defaultdict
filename = input("filename :");
a = defaultdict(int)
with open(filename, 'rb') as f:
    for line in f:
        for w in line:
            a[chr(w)] += 1
for key, value in sorted(a.items()):
    print(key, ":", value)
print(len(a))
