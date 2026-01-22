
# matching
## ensures the string on the left is made up of characters in
## the alnum character class followed by the string name.
## nb: RHS should not be quoted here.
if [[ "filename" =~ ^[[:alnum:]]+name ]]; then
  echo "Match"
fi

## matches the exact pattern "f*" (Does not match in this case)
if [[ "filename" == "f*" ]]; then
  echo "Match"
fi



# arrays
declare -a flags
## initialisation: word-delimited
flags=(--foo --bar='baz')
## appending
flags+=(--greeting="Hello ${name}")
## pass list as words to command
mybinary "${flags[@]}"


# process substitution
## creates a subshell. It allows redirecting from a subshell to a while without putting the while (or any other command) in a subshell.
last_line='NULL'
while read line; do
  if [[ -n "${line}" ]]; then
    last_line="${line}"
  fi
done < <(your_command)

# will output the last non-empty line from your_command
echo "${last_line}"

## readarray ver
# Alternatively use the readarray builtin to read the file into an array, then loop over the array’s contents. NB: one needs to use process substitution with readarray rather than a pipe, but with the advantage that the input generation for the loop is located before it, rather than after.
last_line='NULL'
readarray -t lines < <(your_command)
for line in "${lines[@]}"; do
  if [[ -n "${line}" ]]; then
    last_line="${line}"
  fi
done
echo "${last_line}"

#
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

## getopt flags passthrough
args_parsethrough() {
    # specify the `--` delimiter here
    opts=$(getopt -o "ab:" --long "alpha,bravo:" -- "$@") || exit 2
    eval "set -- $opts"

    local alpha=
    local bravo=
    while true; do
      case "$1" in
        -a|--alpha) alpha=1; shift ;;
        -b|--bravo) bravo="$2"; shift 2 ;;
        --) shift; break ;;
        *) break ;;
      esac
    done
    exec somebin ${bravo+"--bravo ${bravo}"} "$@"
}

# variable expansions (? - not sure of technical phrase)

## modify/set a variable only if it has already been set.
## commonly useful for prepending flags to variables if they have been set, for passing to binaries/scripts.

expand_if_set() {
    local y=
    local x=carrot

    # NB: important to not surround with parentheses, else
    # when unset will resultin an empty string
    mybinary ${x+"--argname ${x}"} ${y+"--othername ${y}"}




}

