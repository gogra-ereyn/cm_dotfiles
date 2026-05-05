compile = gcc -std=c99 -O2 -g -march=native

%: %.c Makefile
	$(compile) $< -o $@

%.s: %.c Makefile
	$(compile) -S -masm=intel -fverbose-asm $< -o $@

%.run: %
	@./$<

.PHONY: %.run

