CC = gcc
CFLAGS = -Wall -Wextra $(shell pkg-config --cflags sdl2) $(shell pkg-config --cflags SDL2_ttf) $(shell pkg-config --cflags SDL2_mixer) -I. -Iinclude $(shell pkg-config --cflags assimp)
LDLIBS = $(shell pkg-config --libs sdl2) $(shell pkg-config --libs SDL2_ttf) $(shell pkg-config --libs SDL2_mixer) -lGL -lGLU -lm $(shell pkg-config --libs assimp)

TARGET = operation-dolphin
SRCDIR = src
OBJDIR = obj

# Source files
SRCS = main.c $(wildcard $(SRCDIR)/*.c)
# Object files
OBJS = $(SRCS:%.c=$(OBJDIR)/%.o)

all: prepare $(TARGET)

prepare:
	@mkdir -p $(OBJDIR) $(OBJDIR)/$(SRCDIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean prepare