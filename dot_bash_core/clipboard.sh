
copy_line() {
    printf '%s' "$READLINE_LINE" | cpy
}

bind -m vi-command -x '"yy": copy_line'
