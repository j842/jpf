MAKEFLAGS+="-j -l $(shell grep -c ^processor /proc/cpuinfo)"

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

include $(ROOT_DIR)/versions.makefile

COM_COLOR   := \033[0;34m
CRE_COLOR   := \033[0;35m
LIN_COLOR   := \033[0;33m
OBJ_COLOR   := \033[0;36m
VER_COLOR	:= \033[0;92m
NO_COLOR    := \033[m

define uniq =
  $(eval seen :=)
  $(foreach _,$1,$(if $(filter $_,${seen}),,$(eval seen += $_)))
  ${seen}
endef

CXX=g++
#CXX=podman/podrun.sh g++

LD=$(CXX)
CXXFLAGS := -g -Wall -std=c++17 
LDFLAGS  := -g
LDLIBS   := -l:libboost_date_time.a -l:libcppunit.a
#DEPFLAGS = -MT $@ -MD -MP -MF $(DEP_DIR)/$*.Td

VER_HEADER=$(SRC_DIR)/version.h

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. Make will incorrectly expand these otherwise.
SRCS := $(shell find $(SRC_DIR) -name '*.cpp' -or -name '*.c' -or -name '*.s')
# combine and remove duplicates.
SRCS := $(call uniq,${SRCS})

# String substitution for every C/C++ file.
# As an example, hello.cpp turns into ./build/hello.cpp.o
OBJS := $(SRCS:%=$(BIN_DIR)/%.o)
#OBJS := $(sort $(OBJS))

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIR) -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS := $(INC_FLAGS) -MMD -MP

# The final build step.
$(JPF_EXE): SHOWVER $(OBJS) 
	@printf "%b" "$(LIN_COLOR)Linking   $(OBJ_COLOR)$(@)$(NO_COLOR)\n";
	@$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)
	@echo 

# Build step for C++ source
$(BIN_DIR)/%.cpp.o: %.cpp $(VER_HEADER)
	@printf "%b" "$(COM_COLOR)Compiling $(OBJ_COLOR)$(@)$(NO_COLOR)\n";
	@mkdir -p $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# make version header file.
.PRECIOUS: $(VER_HEADER)
$(VER_HEADER): versions.makefile
	@printf "%b" "$(CRE_COLOR)Creating  $(OBJ_COLOR)$(@)$(NO_COLOR)\n";
	@echo "#define __JPF_VERSION \"${JPF_VERSION}\"" > $@
	@echo "#define __JPF_RELEASE \"${JPF_RELEASE}\"" >> $@
	@echo "#define __INPUT_VERSION (${INPUT_VERSION})" >> $@

SHOWVER:
	@printf "%b" "\n$(VER_COLOR)Building $(JPF_FULL_VER)$(NO_COLOR)\n";

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)

# run tests
check: $(JPF_EXE)
	rm -rf ~/.jpf_test
	$(JPF_EXE) -t
	$(JPF_EXE) -c ~/.jpf_test
	$(JPF_EXE) ~/.jpf_testVER_HEADER
DATA_DIR := $(ROOT_DIR)/includes/html/_data
.PHONY: data
data:
	rm -rf ~/.jpf_test
	$(JPF_EXE) -c ~/.jpf_test
	$(JPF_EXE) ~/.jpf_test
	rm $(DATA_DIR)/*
	cp ~/.jpf_test/output/.jekyll/_data/* $(DATA_DIR)
	rm -rf ~/.jpf_test

clean:
	rm -rf $(BIN_DIR)