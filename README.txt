1. Compile with make

2. Execute ./oss -n 5 -s 2 -b 101 -i 4 -o output.txt
First, -h, to describe how it should be run.  Thena -n x option to indicate the maximum total of child processes it will ever create.  Another option, -s, will indicatehow many children should be allowed to exist in the system at the same time.  I suggest the default being of a -n of 4and a -s of 2.  Additional options are -b to indicate the start of the sequence of numbers we are to test for primality,a -i option for the increment between numbers that we test and finally a -o option specifying an output file.  Notethat all of these arguments should have some sensible default values, which should be described if -h is given.

Version  Control:
I pushed all these files in to  github.
/classes/OS/pelaprol/pelaprol.2/log

