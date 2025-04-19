CC = gcc
CFLAGS = -Wall -Wextra $(shell pkg-config --cflags sdl2) $(shell pkg-config --cflags SDL2_ttf) $(shell pkg-config --cflags SDL2_mixer) -I. -Iinclude $(shell pkg-config --cflags assimp)
LDLIBS = $(shell pkg-config --libs sdl2) $(shell pkg-config --libs SDL2_ttf) $(shell pkg-config --libs SDL2_mixer) -lGL -lGLU -lm $(shell pkg-config --libs assimp)

# Release build flags
RELEASE_CFLAGS = -O3 -DNDEBUG -fomit-frame-pointer $(shell pkg-config --cflags sdl2) $(shell pkg-config --cflags SDL2_ttf) $(shell pkg-config --cflags SDL2_mixer) -I. -Iinclude $(shell pkg-config --cflags assimp)
RELEASE_LDFLAGS = -s -Wl,-O1,--sort-common,--as-needed,-z,relro,-z,now

# Dynamic linking with rpath for bundled libraries
RELEASE_LDLIBS = -Wl,-rpath,'$$ORIGIN/lib' $(shell pkg-config --libs sdl2) $(shell pkg-config --libs SDL2_ttf) $(shell pkg-config --libs SDL2_mixer) -lGL -lGLU -lm $(shell pkg-config --libs assimp)

TARGET = operation-dolphin
RELEASE_TARGET = $(TARGET)-release
SRCDIR = src
OBJDIR = obj
RELEASEDIR = Release
RELEASE_OBJDIR = $(OBJDIR)/release

# Source files
SRCS = main.c $(wildcard $(SRCDIR)/*.c)
# Object files
OBJS = $(SRCS:%.c=$(OBJDIR)/%.o)
RELEASE_OBJS = $(SRCS:%.c=$(RELEASE_OBJDIR)/%.o)

# SDL2 Libraries needed (add more if required)
SDL_LIBS = \
	libSDL2-2.0.so.0 \
	libSDL2_ttf-2.0.so.0 \
	libSDL2_mixer-2.0.so.0

# Release assets
RELEASE_ASSETS = \
	fonts \
	textures \
	media \
	music \
	models \
	LICENSE \
	README.md

all: prepare $(TARGET)

release: prepare-release $(RELEASE_TARGET) package-release

prepare:
	@mkdir -p $(OBJDIR) $(OBJDIR)/$(SRCDIR)

prepare-release:
	@mkdir -p $(RELEASE_OBJDIR) $(RELEASE_OBJDIR)/$(SRCDIR)
	@mkdir -p $(RELEASEDIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(RELEASE_TARGET): $(RELEASE_OBJS)
	$(CC) $(RELEASE_CFLAGS) $(RELEASE_LDFLAGS) -o $@ $^ $(RELEASE_LDLIBS)
	@echo "Release build created: $(RELEASE_TARGET)"

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(RELEASE_OBJDIR)/%.o: %.c
	$(CC) $(RELEASE_CFLAGS) -c $< -o $@

# Find the system library locations
find-sdl-libs:
	@echo "Finding required SDL2 libraries..."
	@for lib in $(SDL_LIBS); do \
		location=$$(ldconfig -p | grep $$lib | awk '{print $$4}' | head -n1); \
		if [ -z "$$location" ]; then \
			echo "⚠️  Warning: $$lib not found in system"; \
		else \
			echo "✓ Found $$lib: $$location"; \
		fi; \
	done

package-release: $(RELEASE_TARGET)
	@echo "Packaging release..."
	@rm -rf $(RELEASEDIR)/$(TARGET)
	@mkdir -p $(RELEASEDIR)/$(TARGET)
	@mkdir -p $(RELEASEDIR)/$(TARGET)/lib
	@cp $(RELEASE_TARGET) $(RELEASEDIR)/$(TARGET)/$(TARGET)
	@echo "Copying required libraries..."
	@for lib in $(SDL_LIBS); do \
		location=$$(ldconfig -p | grep $$lib | awk '{print $$4}' | head -n1); \
		if [ -n "$$location" ]; then \
			echo "Copying $$lib"; \
			cp -L "$$location" $(RELEASEDIR)/$(TARGET)/lib/; \
		fi; \
	done
	@for asset in $(RELEASE_ASSETS); do \
		if [ -e "$$asset" ]; then \
			cp -r "$$asset" $(RELEASEDIR)/$(TARGET)/; \
		fi \
	done
	
	@# Create startup script
	@echo '#!/bin/bash' > $(RELEASEDIR)/$(TARGET)/start.sh
	@echo 'cd "$(dirname "$$0")"' >> $(RELEASEDIR)/$(TARGET)/start.sh
	@echo 'export LD_LIBRARY_PATH="$$PWD/lib:$$LD_LIBRARY_PATH"' >> $(RELEASEDIR)/$(TARGET)/start.sh
	@echo './$(TARGET)' >> $(RELEASEDIR)/$(TARGET)/start.sh
	@chmod +x $(RELEASEDIR)/$(TARGET)/start.sh
	
	@cd $(RELEASEDIR) && zip -r $(TARGET)-$(shell date +%Y%m%d).zip $(TARGET)
	@echo "Release package created: $(RELEASEDIR)/$(TARGET)-$(shell date +%Y%m%d).zip"

clean:
	rm -rf $(OBJDIR) $(TARGET) $(RELEASE_TARGET)

clean-release:
	rm -rf $(RELEASE_OBJDIR) $(RELEASE_TARGET) $(RELEASEDIR)/$(TARGET)

clean-all: clean clean-release
	rm -rf $(RELEASEDIR)/*.zip

.PHONY: all release prepare prepare-release package-release find-sdl-libs clean clean-release clean-all