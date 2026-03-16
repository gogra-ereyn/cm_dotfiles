
# quote vim-word under cursor (alnum/_ boundaries)
quote_inner_word() {
    local line="$READLINE_LINE"
    local pos=$READLINE_POINT
    local start=$pos end=$pos
    local len=${#line}

    # if not on a word char, skip forward to next word
    if ! [[ "${line:pos:1}" =~ [[:alnum:]_] ]]; then
        while (( pos < len )) && ! [[ "${line:pos:1}" =~ [[:alnum:]_] ]]; do
            (( pos++ ))
        done
        (( pos == len )) && return
        start=$pos end=$pos
    fi

    while (( start > 0 )) && [[ "${line:start-1:1}" =~ [[:alnum:]_] ]]; do
        (( start-- ))
    done
    while (( end < len )) && [[ "${line:end:1}" =~ [[:alnum:]_] ]]; do
        (( end++ ))
    done

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

    # if on whitespace, skip forward to next WORD
    if [[ "${line:pos:1}" =~ [[:space:]] || -z "${line:pos:1}" ]]; then
        while (( pos < len )) && [[ "${line:pos:1}" =~ [[:space:]] ]]; do
            (( pos++ ))
        done
        (( pos == len )) && return
        start=$pos end=$pos
    fi

    while (( start > 0 )) && [[ "${line:start-1:1}" != [[:space:]] ]]; do
        (( start-- ))
    done
    while (( end < len )) && [[ "${line:end:1}" != [[:space:]] ]]; do
        (( end++ ))
    done

    local quoted
    quoted=$(printf '%s' "${line:start:end-start}" | qw)

    READLINE_LINE="${line:0:start}${quoted}${line:end}"
    READLINE_POINT=$(( start + ${#quoted} ))
}

bind -m vi-command -x '"qw": quote_inner_word'
bind -m vi-command -x '"qW": quote_inner_WORD'
