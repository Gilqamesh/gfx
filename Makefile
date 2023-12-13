bin = a.out
test_bin = test.out
cc = gcc
common_src = \
	app.c \
	glfw.c \
	debug.c \
	buffer.c \
	system.c \
	glad/src/glad.c
src = \
	$(common_src) \
	main.c
tst_src = \
	$(common_src) \
	test.c
obj = $(src:.c=.o)
test_obj = $(tst_src:.c=.o)
dps = $(src:.c=.d)
test_dps = $(tst_src:.c=.d)
cflags = $(shell pkg-config --cflags glfw3) -g -Wall -Wextra -Werror -Iglfw/includes -Iglad/include -O0
lflags = $(shell pkg-config --static --libs glfw3 gl)

.PHONY: all
all: $(bin)

.PHONY: clean
clean:
	- rm -f $(obj) $(test_obj) $(bin) $(test_bin) $(dps) $(test_dps)

.PHONY: re
re: clean
re: all

.PHONY: test 
test: $(test_bin)

$(test_bin): $(test_obj)
	$(cc) -o $(test_bin) $^ $(lflags)

%.o: %.c
	$(cc) -c $< -o $@ -MMD -MP -MF $(<:.c=.d) $(cflags)

$(bin): $(obj)
	$(cc) -o $(bin) $^ $(lflags)

-include $(dps)
