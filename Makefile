bin = a.out
cc = gcc
src = \
	main.c \
	app.c \
	glfw.c \
	debug.c \
	buffer.c \
	system.c \
	glad/src/glad.c
obj = $(src:.c=.o)
dps = $(src:.c=.d)
cflags = $(shell pkg-config --cflags glfw3) -g -Wall -Wextra -Werror -Iglfw/includes -Iglad/include
lflags = $(shell pkg-config --static --libs glfw3 gl)

.PHONY: all
all: $(bin)

.PHONY: clean
clean:
	- rm -f $(obj) $(bin) $(dps)

.PHONY: re
re: clean
re: all

%.o: %.c
	$(cc) -c $< -o $@ -MMD -MP -MF $(<:.c=.d) $(cflags)

$(bin): $(obj)
	$(cc) -o $(bin) $^ $(lflags)

-include $(dps)
