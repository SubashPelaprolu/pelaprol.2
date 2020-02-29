1. Compile with make
  gcc -Wall -ggdb -Werror -c common.c
  gcc -Wall -ggdb -Werror oss.c common.o -o oss -pthread
  gcc -Wall -ggdb -Werror user.c common.o -o user -pthread

2. Execute ./oss
Started child 0 with PID=175318 for number 101 at time 0.0
Started child 1 with PID=175319 for number 105 at time 0.10000
Child 1 was terminated at time 0.60000
Child 0 was terminated at time 0.60000
Started child 2 with PID=175320 for number 109 at time 0.60000
Started child 3 with PID=175321 for number 113 at time 0.70000
Child 2 was terminated at time 0.120000
Child 3 was terminated at time 0.150000
101 is PRIME number
105 is NOT a PRIME number
109 is PRIME number
113 is PRIME number
Finished at 0.150000
