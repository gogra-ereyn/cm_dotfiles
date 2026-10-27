# ss: a CLI tool for generating substrings 

ss is intended for ergonomic string slicing/manipulation via the CLI. It's somewhat tedious to do this using the standard linux shell toolset, and the terminating newlines added by various tools (like `echo` or bash here strings) can make it easy to make command mistakes. `ss`'s range-specifying syntax is inspired by ranges in Rust. 


# Usage

The string-to-slice can either be passed as the second positional argument or provided via stdin.

`ss` takes a range string that specifies how to slice the input string as the first & required positional argument. The syntax of these range strings is inspired by Rusts' ranges. 

The current/default behavior is to trim all trailing newlines from the input (to make it more ergonomic to use common bash tools like `echo` or here strings). This means that all of the following (by default) are equivalent:

- `ss ..-1 hello`
- `echo hello | ss ..-1`    ( despite echo adding a trailing newline )
- `printf hello | ss ..-1`
- `ss ..-1 <<< hello`       ( despite `<<<` adding a trailing newline )

To disable this and preserve all trailing whitespace, use the `-k` or `--keep-whitespace` CLI flags.

## Syntax

Format: `[start]..[[=]end]`

Where
- `start` - (optional) start index, inclusive.  [default: 0]
- `end`   - (optional) end index, exclusive. [default: <input_length>]
- `=`     - (optional) specifies that the range is inclusive of the `end` index.

`start` and `end` can be negative. A negative bound `-n` refers to the `nth` number from the end of the string. This notably means that `-1` referrs to the index of the last character of the input. Adding `=` to the end specifier will make the range inclusive of that index; e.g. `1..=-1` is equivalent to `1..`.

### Basic usage examples: positive indexing

`ss   1..4    hello`         # Output: ell
`ss   ..3     hello`         # Output: hel
`ss   2..     hello`         # Output: llo

### Basic usage examples: negative indexing
`ss   -3..    hello`        # Output: llo 
`ss   ..-2    hello`        # Output: hel
`ss   -4..-1  hello`        # Output: ell


More examples can be seen in `/tests/test.sh`.
