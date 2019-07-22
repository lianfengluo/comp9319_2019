#!/bin/bash

# g++ -print-search-dirs
g++-8 lencode.cpp -o lencode
g++-8 ldecode.cpp -o ldecode
cat space.txt| ./lencode | ./ldecode > space.out
cat space_large.txt| ./lencode | ./ldecode > space_large.out
cat random.txt| ./lencode | ./ldecode > random.out
cat test.txt| ./lencode | ./ldecode > test.out
cat test2.txt| ./lencode | ./ldecode > test2.out
cat aaa.txt| ./lencode | ./ldecode > aaa.out
cat numbers.txt| ./lencode | ./ldecode > numbers.out
diff space.txt space.out
diff space_large.txt space_large.out
diff random.txt random.out
diff test.txt test.out
diff test2.txt test2.out
diff aaa.txt aaa.out
diff numbers.txt numbers.out
