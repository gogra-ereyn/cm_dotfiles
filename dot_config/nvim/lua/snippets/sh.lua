local ls = require("luasnip")
local s = ls.snippet
local t = ls.text_node
local i = ls.insert_node
local f = ls.function_node
local c = ls.choice_node
local fmt = require("luasnip.extras.fmt").fmt

local function code_snippet(trigger, code, filetype)
    filetype = filetype or "sh"

    local lines = {}
    for line in code:gmatch("([^\n]*)\n?") do
        table.insert(lines, line)
    end

    return s(trigger, { t(lines) })
end


local snippets = {
    s("sd", {
        t({
            "declare -r sd=\"$(dirname \"$(readlink -f \"${BASH_SOURCE[0]}\")\")\""
        })
    }),

    s("lsd", {
        t({
            "local -r sd=\"$(dirname \"$(readlink -f \"${BASH_SOURCE[0]}\")\")\""
        })
    }),

    s("rsd", {
        t({
            "declare -r sd=\"$(cd \"$(dirname \"${BASH_SOURCE[0]}\")\" && pwd)\""
        })
    }),
    s("ysd", {
        t({
            "declare -r sd=\"$(dirname \"$(realpath \"${BASH_SOURCE[0]}\")\")"
        })
    }),
    s("orempty", {
        t({
            "${maybeempty:+\"--cc\" \"${maybeempty}\"}}"
        })
    }),

    s({
        trig = "log",
        name = "bash logging",
    }, {
        t({
            "log() {",
            "    local message=\"$1\"",
            "    local level=\"${2:-INFO}\"",
            "    local timestamp=$(date +\"%Y-%m-%d %H:%M:%S\")",
            "    echo \"[$timestamp] [${level^^}] $message\"",
            "}",
            ""
        })
    }),

    s("info", {
        t({
            "info() {",
            "   log $1 info",
            "}",
            ""
        })
    }),

    s("error", {
        t({
            "error() {",
            "   log $1 error",
            "}",
            ""
        })
    }),

    s("abort", {
        t({
            "abort() {",
            "   local msg=\"$1\"",
            "   local -i ec=\"${2-1}\"",
            "   log \"$msg\" fatal",
            "   exit $ec",
            "}"
        })
    }),

    s("issourced", {
        t({ 'is_sourced() {', '' }),
        t({ '    if [ -n "$ZSH_VERSION" ]; then', '' }),
        t({ '        case $ZSH_EVAL_CONTEXT in *:file:*) return 0;; esac', '' }),
        t({ '    elif [ -n "$BASH_VERSION" ]; then', '' }),
        t({ '        (return 0 2>/dev/null) && return 0 || return 1', '' }),
        t({ '    else', '' }),
        t({ '        [ "${BASH_SOURCE[0]}" != "$0" ]', '' }),
        t({ '    fi', '' }),
        t({ '}' })
    }),

    s("scriptdir", {
        t({ 'get_script_dir() {', '' }),
        t({ '    local source="${BASH_SOURCE[0]}"', '' }),
        t({ '    while [ -h "$source" ]; do', '' }),
        t({ '        local dir="$( cd -P "$( dirname "$source" )" && pwd )"', '' }),
        t({ '        source="$(readlink "$source")"', '' }),
        t({ '        [[ $source != /* ]] && source="$dir/$source"', '' }),
        t({ '    done', '' }),
        t({ '    echo "$( cd -P "$( dirname "$source" )" && pwd )"', '' }),
        t({ '}' })
    }),
    s("cmdexists", {
        t({ 'command_exists() {', '' }),
        t({ '    command -v "$1" >/dev/null 2>&1', '' }),
        t({ '}' })
    }),
    s("confirm", {
        t({ 'confirm() {', '' }),
        t({ '    read -r -p "${1:-Are you sure? [y/N]} " response', '' }),
        t({ '    case "$response" in', '' }),
        t({ '        [yY][eE][sS]|[yY])', '' }),
        t({ '            true', '' }),
        t({ '            ;;', '' }),
        t({ '        *)', '' }),
        t({ '            false', '' }),
        t({ '            ;;', '' }),
        t({ '    esac', '' }),
        t({ '}' })
    }),
    s("sourceusage", {
        t({ 'if ! is_sourced; then', '' }),
        t({ '    echo "This script is being run directly"', '' }),
        t({ 'else', '' }),
        t({ '    echo "This script is being sourced"', '' }),
        t({ 'fi' })
    }),

    s("counter", {
        t({ "for i in ${!arr[@]}; do", '' }),
        t({ "echo $i \"${arr[i]}\"" }, ''),
        t({ "done" }, ''),
    }),

    s("assert", {
        t({ "assert() {", "    local actual=\"$1\"", "    local expected=\"$2\"" }),
        t({ "", "    local message=\"${3:-" }), i(1, "Assertion"), t("}\""),
        t({
            "",
            "    if [ \"$actual\" = \"$expected\" ]; then",
            "        echo -e \"\\033[0;32m✓\\033[0m $message\"",
            "        return 0",
            "    else",
            "        echo -e \"\\033[0;31m✗\\033[0m $message\"",
            "        echo -e \"Expected: '$expected'\"",
            "        echo -e \"Actual:   '$actual'\"",
            "        return 1",
            "    fi",
            "}",
            "",
            "assert_eq() {",
            "    assert \"$1\" \"$2\" \"$3\"",
            "}"
        })
    })

}

return snippets
