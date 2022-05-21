MAKEFLAGS+="-j -l $(shell grep -c ^processor /proc/cpuinfo)"

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

include $(ROOT_DIR)/deps/versions.makefile


COM_COLOR   := \033[0;34m
LIN_COLOR   := \033[0;33m
OBJ_COLOR   := \033[0;36m
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
TMPL_DIR:=includes/templates
TMPL:=$(wildcard $(TMPL_DIR)/*)
TMPLSRC:=$(TMPL:includes/templates/%=src/templates/%.cpp)

SUP_DIR:=includes/verbatim
SUPFILES:=$(wildcard $(SUP_DIR)/*)
SUPSRC:=$(SUPFILES:includes/verbatim/%=src/support_files/generate_%.cpp)

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. Make will incorrectly expand these otherwise.
SRCS :=  $(TMPLSRC) $(SUPSRC) $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')
# combine and remove duplicates.
#SRCS := $(sort $(SRCS) $(TMPLSRC) $(SUPSRC))
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
INC_DIRS := $(sort $(INC_DIRS) src/support_files src/templates)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS := $(INC_FLAGS) -MMD -MP

# The final build step.
$(BUILD_DIR)/$(JPF_NAME): $(OBJS)
	@printf "%b" "$(LIN_COLOR)Linking   $(OBJ_COLOR)$(@)$(NO_COLOR)\n";
	@$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

# Build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	@printf "%b" "$(COM_COLOR)Compiling $(OBJ_COLOR)$(@)$(NO_COLOR)\n";
	@mkdir -p $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# make template files
.PRECIOUS:  src/templates/%.cpp
src/templates/%.cpp: $(TMPL_DIR)/% $(ROOT_DIR)/deps/scripts/file2cpp
	@printf "%b" "$(COM_COLOR)Creating  $(OBJ_COLOR)$(@)$(NO_COLOR)\n";
	@mkdir -p $(dir $@)
	@$(ROOT_DIR)/deps/scripts/file2cpp "$<" "$(basename $@)"
src/templates/%.h: src/templates/%.cpp

# make support_files files
.PRECIOUS: src/support_files/generate_%.cpp
src/support_files/generate_%.cpp: $(SUP_DIR)/% $(ROOT_DIR)/deps/scripts/dir2cpp
	@printf "%b" "$(COM_COLOR)Creating  $(OBJ_COLOR)$(@)$(NO_COLOR)\n";
	@mkdir -p $(dir $@)
	@$(ROOT_DIR)/deps/scripts/dir2cpp "$<" "$(basename $@)"
src/support_files/generate_%.h: src/support_files/generate_%.cpp

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(ROOT_DIR)/output
	rm -rf src/templates
	rm -rf src/support_files

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)

check: $(JPF_EXE)
	$(JPF_EXE) -t
	$(JPF_EXE) .
	$(JPF_EXE) -r .
	rm -rf output

deploy:
	@gh auth login --with-token < ~/.github_token
	@gh workflow run deploy_package.yml

setup:
	sudo cp $(ROOT_DIR)/deps/webfsd-jpf/webfsd-jpf /usr/bin
	sudo rm -rf /opt/jpf
	sudo mkdir -p /opt/jpf
	sudo cp -r $(ROOT_DIR)/includes/dpkg_include/* /

images:
	make -C deps/podman

deb: $(BUILD_DIR)/$(JPF_NAME)
	make -C deps -f deploy.makefile deb

upload:
	make -C deps -f deploy.makefile upload
