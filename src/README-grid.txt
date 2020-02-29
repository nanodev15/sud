			  Grid Tests README
			 ===================

Grids marked with a '*' must issue an error message and exit with an error.

                
grid-01: Grid of size 1

grid-02: Grid of size 4

grid-03: Grid of size 9

grid-04: Grid of size 16

grid-05: Grid of size 25

grid-06: Grid of size 36

grid-07: Grid of size 49

grid-08: Grid of size 64

grid-09*: Grid with an unknown character at line 7, column 12

grid-10*: Grid with a character missing at line 9

grid-11*: Grid with an extra character at line 10

grid-12*: Grid with a missing line

grid-13*: Grid with an extra line

grid-14: Grid with an empty line (must be ignored)

grid-15: Grid with a whole line in comment

grid-16: Grid ending directly with an EOF without a '\n' before

grid-17: Grid of size 1 ending directly with an EOF without a '\n' before

grid-18: Grid starting with a comment and an empty line

grid-19*: Grid of size 17

grid-20*: Grid with a commented character (which mustn't be) at line 4

grid-21: Grid with a comment at last line

grid-22*: Grid with a missing character at the last line and finishing
	  directly by an EOF

grid-23*: No grid, only a comment
	  
Tips
----
Running all the tests at once:

for file in grid*; do           \
    echo; echo "Trying $file:"; \
    echo "===============";     \
    ./sudoku $file; done
