
--vim.opt.background = 'dark'
--vim.g.colors_name = 'ereneyic'
--require('lush')(require('lush_theme.ereneyic'))

local lush = require('lush')
local hsl = lush.hsl

local theme = lush(function()
    return {
        -- START CORE
        Normal({ fg = hsl("#c0caf5"), bg = hsl("#251a34") }),
        NormalFloat({ fg = hsl("#c0caf5"), bg = hsl("#16161e") }),
        NormalNC({ fg = hsl("#c0caf5") }),

        Cursor({ fg = hsl("#1a1b26"), bg = hsl("#c0caf5") }),
        CursorLine({ bg = hsl("#292e42") }),
        CursorColumn({ bg = hsl("#292e42") }),
        CursorLineNr({ fg = hsl("#737aa2") }),

        LineNr({ fg = hsl("#777788") }),
        SignColumn({ fg = hsl("#3b4261") }),
        ColorColumn({ bg = hsl("#000000") }),
        Comment({ fg = hsl("#565f89"), italic = true }),
        Constant({ fg = hsl("#ff8994"), bold = false }),
        String({ fg = hsl("#ffcfe8"), bold = false }),
        Character({ link = "Constant" }),
        Number({ fg = hsl("#ff8994"), bold = true }),
        Boolean({ fg = hsl("#ff8994"), bold = false }),
        Float({ link = "Constant" }),
        Identifier({ fg = hsl("#f2e1d1") }),
        Function({ fg = hsl("#6ea2f7"), bold = true }),
        Statement({ link = "Conditional" }),
        Conditional({ fg = hsl("#ff9aef"), bold = false }),
        Repeat({ fg = hsl("#ff9aef"), bold = true }),
        Label({ link = "Conditional" }),
        Operator({ fg = hsl("#ff9aef"), bold = false }),
        Keyword({ fg = hsl("#ff9aef"), bold = true }),
        Exception({ link = "Constant" }),
        PreProc({ link = "Include" }),
        Include({ fg = hsl("#85dff8") }),
        Define({ link = "Include" }),
        Macro({ fg = hsl("#df71e6"), bold = false }),
        PreCondit({ link = "Macro" }),
        Type({ fg = hsl("#85dff8"), bold = true }),
        StorageClass({ link = "Macro" }),
        Structure({ link = "Type" }),
        Typedef({ link = "Type" }),
        Special({ fg = hsl("#85dff8"), bold = false }),
        SpecialChar({ fg = hsl("#ff8994"), bold = false }),
        Tag({ link = "Type" }),
        Delimiter({ fg = hsl("#ffbfe7"), bold = false }),
        SpecialComment({ link = "Type" }),
        Debug({ link = "Function" }),

        -- UI highlights
        Visual({ bg = hsl("#2d59a1") }),
        Search({ fg = hsl("#c0caf5"), bg = hsl("#3d59a1") }),
        IncSearch({ fg = hsl("#000000"), bg = hsl("#ff9e64") }),

        Pmenu({ fg = hsl("#c0caf5"), bg = hsl("#16161e") }),
        PmenuSel({ bg = hsl("#3b4261") }),
        PmenuSbar({ bg = hsl("#16161e") }),
        PmenuThumb({ bg = hsl("#3b4261") }),

        -- Messages and diagnostics
        WarningMsg({ fg = hsl("#e0af68") }),
        ErrorMsg({ fg = hsl("#db4b4b"), bg = hsl("#1a1b26"), bold = true }),
        DiagnosticError({ fg = hsl("#db4b4b") }),
        DiagnosticWarn({ fg = hsl("#e0af68") }),
        DiagnosticInfo({ fg = hsl("#0db9d7") }),
        DiagnosticHint({ fg = hsl("#1abc9c") }),
        ["@comment"]=({ link = "Comment" }),
        ["@conditional"]=({ link = "Conditional" }),
        ["@constant"]=({ link = "Constant" }),
        ["@function"]=({ link = "Function" }),
        ["@keyword"]=({ link = "Keyword" }),
        ["@string"]=({ link = "String" }),
        ["@type"]=({ link = "Type" }),
        ["@variable"]=({ link = "Variable" }),
        NvimSpacing({link = "Normal"}),
        NvimTreePopup({link = "Normal"}),

        -- TODOs
        Todo({fg = hsl("#1a1b26"), bg = hsl("#e0af68"), bold = true}),
        luaTodo({link = "Todo"}),
        ["@text.todo"] = {link = "Todo"},
        ["@text.warning"] = {link = "Todo"},
        TreesitterContext = {bg = "#16161e"},



    }
end)

return theme
