# apc - arbitrary precision calculator shell command
Usage:
- `apc` starts repl mode
- `apc "..."` evaluates the string and prints the result

Currently supports:
- Operations: `+`,`-`,`*`, `( )`
- Arbitrary precision arithmetic
- Numbers are stored in any base from 2-36
- Explicit base operator `_`
- Base conversion operator `#`

Will support:
- Digit separator `'`
- Identifiers and variable assignment
- More operations
- Unit conversion
 
Examples:
- `(0110'1001 _ 2) # 10` - convert 105 in binary to base 10
