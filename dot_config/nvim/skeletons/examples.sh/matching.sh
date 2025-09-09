# This ensures the string on the left is made up of characters in
# the alnum character class followed by the string name.
# Note that the RHS should not be quoted here.
if [[ "filename" =~ ^[[:alnum:]]+name ]]; then
  echo "Match"
fi

# This matches the exact pattern "f*" (Does not match in this case)
if [[ "filename" == "f*" ]]; then
  echo "Match"
fi

# An array is assigned using parentheses, and can be appended to
# with +=( … ).
declare -a flags
flags=(--foo --bar='baz')
flags+=(--greeting="Hello ${name}")
mybinary "${flags[@]}"


# Using process substitution also creates a subshell. However, it allows redirecting from a subshell to a while without putting the while (or any other command) in a subshell.
last_line='NULL'
while read line; do
  if [[ -n "${line}" ]]; then
    last_line="${line}"
  fi
done < <(your_command)

# This will output the last non-empty line from your_command
echo "${last_line}"

## readarray ver
# Alternatively, use the readarray builtin to read the file into an array, then loop over the array’s contents. Notice that (for the same reason as above) you need to use a process substitution with readarray rather than a pipe, but with the advantage that the input generation for the loop is located before it, rather than after.

last_line='NULL'
readarray -t lines < <(your_command)
for line in "${lines[@]}"; do
  if [[ -n "${line}" ]]; then
    last_line="${line}"
  fi
done
echo "${last_line}"

# PIPESATUS
tar -cf - ./* | ( cd "${DIR}" && tar -xf - )
return_codes=( "${PIPESTATUS[@]}" )
if (( return_codes[0] != 0 )); then
  do_something
fi
if (( return_codes[1] != 0 )); then
  do_something_else
fi



# builtins
substitution="${string/#foo/bar}"

if [[ "${string}" =~ foo:(\d+) ]]; then
  extraction="${BASH_REMATCH[1]}"
fi

#rematch2
text="name=value with spaces"
[[ $text =~ ^([a-z]+)=(.*)$ ]]
echo complete match: "${BASH_REMATCH[0]}"
echo name: "${BASH_REMATCH[1]}"
echo value: "${BASH_REMATCH[2]}"


substrings() {
string=01234567890abcdefgh
echo ${string:7}
# 7890abcdefgh

$ echo ${string:7:-2}
# 7890abcdef
}
