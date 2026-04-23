CXX     := g++
TARGET  := ./bin/unpacker
OBJDIR  := obj

ROOTCFLAGS  := $(shell root-config --cflags)
ROOTLIBS    := $(shell root-config --libs)
TCLINCLUDES := -I/usr/include/tcl8.6
TCLLIBS     := -L/usr/lib/x86_64-linux-gnu \
                -ltk8.6 -lX11 -lXss -lXext -lXft \
                -lfontconfig -lfreetype \
                -lpthread -ldl -lz -lm \
                -ltcl8.6

CXXFLAGS := -g -O4 -Wall $(ROOTCFLAGS)
CPPFLAGS := -I./include -I./modules/include $(TCLINCLUDES)
DEPFLAGS := -MMD -MP
LDFLAGS  := $(ROOTLIBS) $(TCLLIBS)

SOURCES := \
    main.cxx              \
    src/EventData.cpp     \
    src/EventReader.cpp   \
    src/EventParser.cpp   \
    src/InputAction.cpp   \
    src/OutputAction.cpp  \
    src/StackAction.cpp   \
    src/TreeAction.cpp    \
    src/DisplayAction.cpp \
    src/Unpacker.cpp      \
    modules/src/ADCV775.cpp \
    modules/src/ADCV785.cpp \
    modules/src/ADCV792.cpp \
    modules/src/ADCV862.cpp \
    modules/src/TDC1190.cpp \
    modules/src/V2740.cpp

OBJECTS := $(patsubst modules/src/%.cpp, $(OBJDIR)/%.o, \
           $(patsubst src/%.cpp,         $(OBJDIR)/%.o, \
           $(patsubst main.cxx,          $(OBJDIR)/main.o, $(SOURCES))))
DEPS    := $(OBJECTS:.o=.d)
TOTAL   := $(words $(OBJECTS))

# ANSI colors
GREEN  := \033[0;32m
CYAN   := \033[0;36m
YELLOW := \033[0;33m
BOLD   := \033[1m
RESET  := \033[0m

# I like coloring things lol sorry
STEP := 0
define advance
$(eval STEP := $(shell echo $$(( $(STEP) + 1 ))))
endef
define pct
$(shell echo $$(( $(1) * 100 / $(2) )))
endef

.PHONY: all clean distclean

all: $(TARGET)
	@printf "\n$(GREEN)$(BOLD)Build complete: $(RESET)$(TARGET)\n\n"

$(TARGET): $(OBJECTS) | bin/
	@printf "$(CYAN)$(BOLD)[100%%]$(RESET)  Linking $(BOLD)$(TARGET)$(RESET)\n"
	@$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJDIR) bin/:
	@mkdir -p $@

$(OBJDIR)/%.o: src/%.cpp | $(OBJDIR)
	$(call advance)
	@printf "$(GREEN)[%3d%%]$(RESET)  Building class $<\n" "$(call pct,$(STEP),$(TOTAL))"
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEPFLAGS) -c $< -o $@

$(OBJDIR)/%.o: modules/src/%.cpp | $(OBJDIR)
	$(call advance)
	@printf "$(GREEN)[%3d%%]$(RESET)  Building class $<\n" "$(call pct,$(STEP),$(TOTAL))"
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEPFLAGS) -c $< -o $@

$(OBJDIR)/main.o: main.cxx | $(OBJDIR)
	$(call advance)
	@printf "$(GREEN)[%3d%%]$(RESET)  Building executable $<\n" "$(call pct,$(STEP),$(TOTAL))"
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEPFLAGS) -c $< -o $@

clean:
	@printf "$(YELLOW)Cleaning build artifacts$(RESET)\n"
	@rm -f $(OBJECTS) $(DEPS) $(TARGET)

distclean: clean
	@printf "$(YELLOW)Removing build directories...$(RESET)\n"
	@rm -rf $(OBJDIR) bin/

-include $(DEPS)
