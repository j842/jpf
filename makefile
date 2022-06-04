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

BUILD_DIR := build
SRC_DIRS := src

CXX=g++
#CXX=podman/podrun.sh g++

LD=$(CXX)
CXXFLAGS := -g -Wall -std=c++17 
LDFLAGS  := -g
LDLIBS   := -l:libboost_date_time.a -l:libcppunit.a
#DEPFLAGS = -MT $@ -MD -MP -MF $(DEP_DIR)/$*.Td

SUP_DIR:=includes/verbatim
SUPFILES:=$(wildcard $(SUP_DIR)/*)
SUPSRC:=$(SUPFILES:includes/verbatim/%=src/support_files/generate_%.cpp)

ALLSUPFILES := $(shell find $(SUP_DIR) -name '*')

VER_HEADER=src/version.h

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. Make will incorrectly expand these otherwise.
SRCS := $(SUPSRC) $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')
# combine and remove duplicates.
SRCS := $(call uniq,${SRCS})

# String substitution for every C/C++ file.
# As an example, hello.cpp turns into ./build/hello.cpp.o
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
#OBJS := $(sort $(OBJS))

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_DIRS := $(sort $(INC_DIRS) src/support_files)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS := $(INC_FLAGS) -MMD -MP

# The final build step.
$(BUILD_DIR)/$(JPF_NAME): SHOWVER $(VER_HEADER) $(OBJS) 
	@printf "%b" "$(LIN_COLOR)Linking   $(OBJ_COLOR)$(@)$(NO_COLOR)\n";
	@$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)
	@echo 

# Build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	@printf "%b" "$(COM_COLOR)Compiling $(OBJ_COLOR)$(@)$(NO_COLOR)\n";
	@mkdir -p $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# make support_files files
.PRECIOUS: src/support_files/generate_%.cpp
src/support_files/generate_%.cpp: $(SUP_DIR)/% $(ROOT_DIR)/deps/scripts/dir2cpp $(ALLSUPFILES)
	@printf "%b" "$(CRE_COLOR)Creating  $(OBJ_COLOR)$(@)$(NO_COLOR)\n";
	@mkdir -p $(dir $@)
	@$(ROOT_DIR)/deps/scripts/dir2cpp "$<" "$(basename $@)"
src/support_files/generate_%.h: src/support_files/generate_%.cpp

# make version header file.
.PRECIOUS: $(VER_HEADER)
$(VER_HEADER): versions.makefile
	@printf "%b" "$(CRE_COLOR)Creating  $(OBJ_COLOR)$(@)$(NO_COLOR)\n";
	@echo "#define __JPF_VERSION \"${JPF_VERSION}\"" > $@
	@echo "#define __JPF_RELEASE \"${JPF_RELEASE}\"" >> $@
	@echo "#define __INPUT_VERSION (${INPUT_VERSION})" >> $@

# clean up any temp/build files.
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -rf src/support_files
	rm $(VER_HEADER)

.PHONY: SHOWVER
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
	$(JPF_EXE) ~/.jpf_test
	$(JPF_EXE) -r ~/.jpf_test
	rm -rf ~/.jpf_test

# use a GitHub Action to build and upload the debian package to packagecloud on Ubuntu 20.04 (i.e. release to the world!)
deploy:
	@gh auth login --with-token < ~/.github_token
	@gh workflow run Build_and_Deploy_JPF.yml

# create the podman (docker) images for building the debian package.
images:
	make -C deps/podman

# build the debian package.
deb: $(BUILD_DIR)/$(JPF_NAME)
	make -C deps -f deploy.makefile deb

DATA_DIR := $(ROOT_DIR)/includes/dpkg_include/opt/jpf/html/_data
.PHONY: data
data:
	rm -rf ~/.jpf_test
	$(JPF_EXE) -c ~/.jpf_test
	$(JPF_EXE) ~/.jpf_test
	rm $(DATA_DIR)/*
	cp ~/.jpf_test/output/.jekyll/_data/* $(DATA_DIR)
	rm -rf ~/.jpf_test
	