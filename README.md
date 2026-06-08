# Setup, compilation
- Install and build Nauty.
- Copy the folder into `./include/` and rename it to `nauty` (or make a symbolic link).
  (final folder structure should have eg. `./include/nauty/nauty.a`)
- Call `make`.

# Usage
There are three modes:
## Encode
`./symencode encode -i graphs.g6 -a automorphisms.aut -o graphs.ssg`
Will read a file of graphs in either graph6 or sparse6 format (detected
automatically) and a list of corresponding automorphisms and write the encoded
strings into graphs.ssg.
Automorphisms file should have the same number of lines as the input graph
file, each line should contain an automorphism as a permutation of the integers
from 1 to n in list notation, separated by commas (eg. "2,3,1").

## Decode
`./symencode decode -i graphs.ssg -o graphs.s6 -s`
Will read an encoded file and write the graphs in graph6 or sparse6 format.
The flag `-s` will write in sparse6, while `-d` will write in graph6.
Note that a encoded and decoded graph will only be isomorphic to the original,
not necessarily identical.

## Test
`./symencode test -i graphs.ssg -I graphs.s6`
Tests if the graphs on the same lines in the files are isomorphic.
The supported formats are graph6, sparse6 and this format.

# Description of procedure
Can be found in symmetric\_graph\_encoding.pdf.
