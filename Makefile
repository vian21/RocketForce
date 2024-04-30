#
# RocketForce build orchastrator
# Patrick Igiraneza, April 2024
#

# Source code
IMGUI_DIR = imgui
SOURCES = src/main.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer2.cpp

# Compiler options
CXX := clang++
CXXFLAGS += -std=c++11 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS += -g -Wall -Wformat
LIBS = 

# Target executable
OUTPUT_DIR := bin
OBJS = $(addprefix $(OUTPUT_DIR)/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
TARGET_BIN = $(OUTPUT_DIR)/simulate

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	ifeq ($(WITH_EXTRA_WARNINGS), 1)
		CXXFLAGS += -Wextra -Wpedantic
		ifeq ($(shell $(CXX) -v 2>&1 | grep -c "clang version"), 1)
			CXXFLAGS += -Wshadow -Wsign-conversion
		endif
	endif
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs`

	CXXFLAGS += `sdl2-config --cflags`
	CXXFLAGS += -I/usr/local/include -I/opt/local/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	ifeq ($(WITH_EXTRA_WARNINGS), 1)
		CXXFLAGS += -Wextra -Wpedantic
	endif
	LIBS += -limm32
	CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

$(OUTPUT_DIR)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUTPUT_DIR)/%.o: $(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUTPUT_DIR)/%.o: $(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUTPUT_DIR)/%.o:$(IMGUI_DIR)/misc/freetype/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(TARGET_BIN)
	@echo "✅ Build complete for $(ECHO_MESSAGE)"

$(TARGET_BIN): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

run: $(TARGET_BIN)
	./$^

clean:
	rm -f $(EXE) $(OBJS)
