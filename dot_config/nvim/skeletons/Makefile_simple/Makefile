cc = gcc
cflags = -std=c99 -O3 -march=native -Wall -Wextra -MMD

objs = main.o foo.o bar.o

prog: $(objs)
	$(cc) $(objs) -o $@

%.o: %.c
	$(cc) $(cflags) -c $< -o $@

-include $(objs:.o=.d)
