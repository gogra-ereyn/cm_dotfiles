local ls = require("luasnip")
local s = ls.snippet
local t = ls.text_node
local i = ls.insert_node
local f = ls.function_node
local c = ls.choice_node
local fmt = require("luasnip.extras.fmt").fmt

local function code_snippet(trigger, code, filetype)
    filetype = filetype or "c"

    local lines = {}
    for line in code:gmatch("([^\n]*)\n?") do
        table.insert(lines, line)
    end

    return s(trigger, { t(lines) })
end

local snippets = {


    code_snippet("usage", [[
int usage(char *name, int rc)
{
	fprintf(stderr, "Usage: %s [options]\n", name);
	fprintf(stderr, "Options:\n");

	fprintf(stderr, "\t-h, --help\n");
	fprintf(stderr, "\t\tShow this help message\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "\t-i, --input FILE\n");
	fprintf(stderr, "\t\tSpecify input file\n");
	fprintf(stderr, "\t\t[default: stdin]\n");
	fprintf(stderr, "\t\t[env: INPUT_FILE=%s]\n", envstr("INPUT_FILE"));
	fprintf(stderr, "\n");
	return rc;
}]]),

    code_snippet("max", [[
#define max(a, b) ((a) > (b) ? (a) : (b))
]]),

    code_snippet("min", [[
#define min(a, b) ((a)  (b) ? (a) : (b))
]]),

    code_snippet("ALEN", [[
#define ALEN(x) (sizeof(x) / sizeof(*x))
]]),

    code_snippet("MATRIX_TO_PTRS", [[
    (int *[sizeof(mat) / sizeof((mat)[0])]){ \
        [0 ... (sizeof(mat)/sizeof((mat)[0]) - 1)] = (mat)[0] \
    }
]]),

    code_snippet("assert_eq", [[
#define assert_eq(left, right, ...)                                            \
	do {                                                                   \
		__typeof__(left) _left_val = (left);                           \
		__typeof__(right) _right_val = (right);                        \
		if (_left_val != _right_val) {                                 \
			fprintf(stderr, "Assertion failed: %s == %s\n", #left, \
				#right);                                       \
			fprintf(stderr, "  Left:  %lld (0x%llx)\n",            \
				(long long)_left_val,                          \
				(unsigned long long)_left_val);                \
			fprintf(stderr, "  Right: %lld (0x%llx)\n",            \
				(long long)_right_val,                         \
				(unsigned long long)_right_val);               \
			fprintf(stderr, "  File: %s, Line: %d\n", __FILE__,    \
				__LINE__);                                     \
			if (sizeof(#__VA_ARGS__) > 2) {                        \
				fprintf(stderr, "  Message: " __VA_ARGS__);    \
				fputc('\n', stderr);                           \
			}                                                      \
			abort();                                               \
		}                                                              \
	} while (0)
]]),

    code_snippet("or_ret", [[
#define or_ret(expr)       \
	do {                   \
		int rc = (expr);   \
		if (rc != 0)       \
			return rc;     \
	} while (0)
]]),

    code_snippet("static_assert", [[
#define static_assert(COND,MSG) typedef char static_assertion_##MSG[(COND)?1:-1]
]]),

    code_snippet("stridemax", [[
uint64_t get_max(uint64_t *times, unsigned count, unsigned stride)
{
	uint64_t max = 0;
	while (count--) {
		if (*times > max)
			max = *times;
			times = (uint64_t *)((void *)times + stride);
		}
		return max;
}]]),

    code_snippet("t_error", [[
void t_error(int status, int errnum, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	vfprintf(stderr, format, args);
	if (errnum)
		fprintf(stderr, ": %s", strerror(errnum));
	fprintf(stderr, "\n");
	va_end(args);
	exit(status);
}
]]),


    code_snippet("pmatrix", [[
void pmatrix(int *input, int count, int m)
{
	int max_width = 0;
	for (int i = 0; i < count; ++i) {
		int width = snprintf(NULL, 0, "%d", input[i]);
		if (width > max_width) {
			max_width = width;
		}
	}

	fprintf(stderr, "[pmatrix: %d x %d] \n\n", count/m, m);
	int i = 0;
	for (; i < count; ++i) {
		fprintf(stderr, "%-*d", max_width+3, input[i]);
		if ((i + 1) % m == 0) {
			fprintf(stderr, "\n");
		}
	}
	if (count % m != 0) {
		fprintf(stderr, "\n");
	}
}]]),

    code_snippet("parray", [[
void parray(int *input, int count)
{
	fprintf(stderr, "[parray: %d] ", count);
	int i = 0;

	for (; i < count; ++i) {
		fprintf(stderr, "%d", input[i]);
		if (i != count-1)
			fprintf(stderr, ", ");
	}
	fprintf(stderr, "\n");

}]]),


    s("now_ms", fmt([[
uint64_t now_ns(void)
{{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ((uint64_t)ts.tv_sec * 1000) + (uint64_t)ts.tv_nsec / 1000);
}}]], {})),

    s("ENVSTR", fmt([[
#define ENVSTR(name) (getenv(name) ? getenv(name) : "")
]], {})),

    s("envstr", fmt([[
har *envstr(char *name) {{
	char *value = getenv(name);
	return value ? value : "";
}}]], {})),

    s("now_ns", fmt([[
uint64_t now_ns(void)
{{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ((uint64_t)ts.tv_sec * 1000000000) + ts.tv_nsec;
}}]], {})),


    s("askusr", fmt([[
int askusr(char *qs, uint8_t *resp, uint8_t flags)
{{
	printf("%s ", qs);
	fflush(stdout);
	if (!readline(out) && !flags) {{
		fprintf(stderr, "Failed to read response for required arg: %s", strerror(errno));
		return -1;
	}}
	return 0;
}}
]], {})),


    s("avg", {
        t({
            "int safe_average(int a, int b) {",
            "\treturn (a & b) + ((a ^ b) >> 1);",
            "}",
        })
    }),

    s("lopt", {
        t({
            "static struct option long_options[] = {",
            "\t{\"help\", no_argument, 0, 'h'},",
            "\t{\"input\", required_argument, 0, 'i'},",
            "\t{\"output\", required_argument, 0, 'o'},",
            "\t{0, 0, 0, 0}",
            "};",
            "",
            "int option_index = 0;",
            "int c;",
            "",
            "while ((c = getopt_long(argc, argv, \"hi:o:\", long_options, &option_index)) != -1) {",
            "switch (c) {",
            "case 'h':",
            "\tfprintf(stderr,\"Usage: %s [options]\\n\", argv[0]);",
            "\tfprintf(stderr,\"Options:\\n\");",
            "\tfprintf(stderr,\"  -h, --help\\t\\tShow this help message\\n\");",
            "\tfprintf(stderr,\"  -i, --input FILE\\tSpecify input file\\n\");",
            "\tfprintf(stderr,\"  -o, --output FILE\\tSpecify output file\\n\");",
            "\tfreturn 0;",
            "case 'i':",
            "\tprintf(\"Input file: %s\\n\", optarg);",
            "\tbreak;",
            "case 'o':",
            "\tprintf(\"Output file: %s\\n\", optarg);",
            "\tbreak;",
            "case '?':",
            "\t// getopt_long already printed an error message",
            "\treturn 1;",
            "default:",
            "\tusage(*argv, 1);",
            "}",
            "}",
            "",
            "if (optind < argc) {",
            "\tprintf(\"non-option ARGV-elements: \");",
            "\twhile (optind < argc)",
            "\t\tprintf(\"%s \", argv[optind++]);",
            "\tprintf(\"\\n\");",
            "}",
            ""
        })
    }),

    s("cmainlong", {
        t({
            "#include <stdio.h>",
            "#include <stdlib.h>",
            "#include <getopt.h>",
            "",
            "int parse_opts(int argc, char **argv);",
            "",
            "int main(int argc, char **argv) {",
            "\tint result = parse_opts(argc, argv);",
            "\tif (result != 0) {",
            "\t\treturn result;",
            "\t}",
            "",
            "\t// code sth you animal",
            "\tprintf(\"Main running for `%s`...\\n\", *argv);",
            "",
            "\treturn 0;",
            "}"
        })
    }),

    s("cmain", {
        t({
            "#include <stdio.h>",
            "#include <stdlib.h>",
            "#include <unistd.h>",
            "",
            "int main(int argc, char **argv) {",
            "\tint opt;",
            "\tchar *ifile = NULL;",
            "\tchar *ofile = NULL;",
            "\twhile ((opt = getopt(argc, argv, \"hi:o:\")) != -1) {",
            "\t\tswitch (opt) {",
            "\t\tcase 'h':",
            "\t\t\tprintf(\"Usage: %s [-h] [-i input_file] [-o output_file]\\n\", argv[0]);",
            "\t\t\treturn 0;",
            "\t\tcase 'i':",
            "\t\t\tifile = optarg;",
            "\t\t\tbreak;",
            "\t\tcase 'o':",
            "\t\t\tofile = optarg;",
            "\t\t\tbreak;",
            "\t\tdefault:",
            "\t\t\tfprintf(stderr, \"Usage: %s [-h] [-i input_file] [-o output_file]\\n\", argv[0]);",
            "\t\t\treturn 1;",
            "\t\t}",
            "\t}",
            "",
            "\tif (ifile) printf(\"Input file: %s\\n\", ifile);",
            "\tif (ofile) printf(\"Output file: %s\\n\", ofile);",
            "",
            "\treturn 0;",
            "}"
        })
    }),

    s("creadfile", {
        t({ "FILE *fp = fopen(" }),
        i(1, "filename"),
        t({ ", \"r\");",
            "if (fp == NULL) {",
            "perror(\"Error opening file\");",
            "\treturn 1;",
            "}",
            "",
            "char *line = NULL;",
            "size_t len = 0;",
            "ssize_t read;",
            "",
            "while ((read = getline(&line, &len, fp)) != -1) {",
            "\t" }),
        i(2, "printf(\"%s\", line);"),
        t({ "",
            "}",
            "",
            "fclose(fp);",
            "if (line)",
            "\tfree(line);" }),
    }),

    s("cerr", {
        t({ "#define ERROR_CHECK(condition, message) \\",
            "\tdo { \\",
            "\t\tif (condition) { \\",
            "\t\t\tfprintf(stderr, \"Error: %s\\n\", message); \\",
            "\t\t\t" }),
        c(1, {
            t("exit(1);"),
            t("return 1;"),
            t("goto cleanup;"),
        }),
        t({ " \\",
            "\t\t} \\",
            "\t} while (0)" }),
    }),

    s("calloc_safe", {
        i(1, "int"),
        t(" *"),
        i(2, "arr"),
        t(" = calloc("),
        i(3, "num_elements"),
        t(", sizeof("),
        f(function(args) return args[1][1] end, { 1 }),
        t({ "));",
            "if (" }),
        f(function(args) return args[1][1] end, { 2 }),
        t({ " == NULL) {",
            "\tfprintf(stderr, \"Memory allocation failed\\n\");",
            "\treturn EXIT_FAILURE;",
            "}" }),
    }),

    s("salloc", {
        t({ "void *ptr = calloc(" }), i(1, "num_elements"), t({ ", sizeof(" }), i(2, "type"), t({ "));",
        "if (ptr == NULL) {",
        "\tfprintf(stderr, \"failed to alloc\\n\");",
        "\texit(EXIT_FAILURE);",
        "}" }),
    }),

    s("cfgets", {
        t({ "char buffer[1024];", "" }),
        t({ "while (fgets(buffer, sizeof(buffer), stdin) != NULL) {", "    " }),
        i(1, "int value"),
        t({ ";", "    if (sscanf(buffer, \"" }),
        i(2, "%d"),
        t({ "\", &" }),
        i(3, "value"),
        t({ ") == 1) {", "        // Process the input", "        " }),
        i(4, "printf(\"Processed value: %d\\n\", value);"),
        t({ "", "    } else {", "        fprintf(stderr, \"Invalid input\\n\");", "    }", "}" }),
    }),

    s("cenv", {
        t("char *"),
        i(1, "verbose"),
        t(" = getenv(\""),
        i(2, "LDRV"),
        t("\");"),
        t({ "", "if (" }),
        f(function(args) return args[1][1] end, { 1 }),
        t(" && strcmp("),
        f(function(args) return args[1][1] end, { 1 }),
        t(", \"1\") == 0)"),
        t({ "", "    *flags |= " }),
        i(3, "VERBOSE"),
        t(";"),
    }),

    s("pint", {
        t({
            "int parse_int(char *input)",
            "{",
            "\tint res = 0;",
            "\twhile (*input)",
            "\t\tres = res * 10 + (*input++ - '0');",
            "\treturn res;",
            "}"
        }),
        t({ "", "" })
    }),
}

return snippets
