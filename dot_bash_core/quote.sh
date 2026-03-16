
# quote vim-word under cursor (alnum/_ boundaries)
quote_inner_word() {
    local line="$READLINE_LINE"
    local pos=$READLINE_POINT
    local start=$pos end=$pos
    local len=${#line}

    while (( start > 0 )) && [[ "${line:start-1:1}" =~ [[:alnum:]_] ]]; do
        (( start-- ))
    done
    while (( end < len )) && [[ "${line:end:1}" =~ [[:alnum:]_] ]]; do
        (( end++ ))
    done
    (( start == end )) && return

    local quoted
    quoted=$(printf '%s' "${line:start:end-start}" | qw)

    READLINE_LINE="${line:0:start}${quoted}${line:end}"
    READLINE_POINT=$(( start + ${#quoted} ))
}

# quote WORD under cursor (whitespace boundaries)
quote_inner_WORD() {
    local line="$READLINE_LINE"
    local pos=$READLINE_POINT
    local start=$pos end=$pos
    local len=${#line}

    while (( start > 0 )) && [[ "${line:start-1:1}" != [[:space:]] ]]; do
        (( start-- ))
    done
    while (( end < len )) && [[ "${line:end:1}" != [[:space:]] ]]; do
        (( end++ ))
    done
    (( start == end )) && return

    local quoted
    quoted=$(printf '%s' "${line:start:end-start}" | qw)

    READLINE_LINE="${line:0:start}${quoted}${line:end}"
    READLINE_POINT=$(( start + ${#quoted} ))
}

bind -m vi-command -x '"qiw": quote_inner_word'
bind -m vi-command -x '"qIw": quote_inner_WORD'
