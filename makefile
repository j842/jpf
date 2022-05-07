#---------------------------------

JPF_VERSION := 0.0.10
JPF_RELEASE := 15
INPUT_VERSION := 6
#---------------------------------

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

SRC_DIR := src
OBJ_DIR := .obj
DEP_DIR := .dep
BIN_DIR := bin
INP_DIR := input

JPF_NAME	:= jpf
JPF_DEB 	:= $(BIN_DIR)/$(JPF_NAME)-$(JPF_VERSION)-$(JPF_RELEASE)-any.deb
JPF_SCRIPT 	:= $(BIN_DIR)/$(JPF_NAME)_script.sh
JPF_EXE 	:= $(BIN_DIR)/$(JPF_NAME)

SRC := $(wildcard $(SRC_DIR)/*.cpp)
#HDR := $(wildcard $(SRC_DIR)/*.h)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEP := $(SRC:$(SRC_DIR)/%.cpp=$(DEP_DIR)/%.d)

VER_HEADER := $(SRC_DIR)/version.h

COM_COLOR   = \033[0;34m
LIN_COLOR   = \033[0;33m
OBJ_COLOR   = \033[0;36m
NO_COLOR    = \033[m
COM_STRING   = "Compiling"
LIN_STRING   = "Linking"

#---------------------------------

# sudo apt-get install libboost-date-time-dev libcppunit-dev
CXX=g++
LD=g++
CXXFLAGS := -g -Wall -std=c++17 
LDFLAGS  := -g
LDLIBS   := -l:libboost_date_time.a -l:libcppunit.a
DEPFLAGS = -MT $@ -MD -MP -MF $(DEP_DIR)/$*.Td

PRECOMPILE =
POSTCOMPILE = mv -f $(DEP_DIR)/$*.Td $(DEP_DIR)/$*.d

COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) -c -o $@
LINK.o = $(LD) $(LDFLAGS) -o $@

.PHONY: all clean deb upload webdeb

all: $(JPF_EXE)

$(VER_HEADER): makefile
	@echo "Generating $@"
	@echo "#define __JPF_VERSION \"${JPF_VERSION}\"" > $@
	@echo "#define __JPF_RELEASE \"${JPF_RELEASE}\"" >> $@
	@echo "#define __INPUT_VERSION (${INPUT_VERSION})" >> $@

$(JPF_EXE): $(VER_HEADER) $(OBJ) | $(BIN_DIR) $(OBJ_DIR) $(DEP_DIR) 
	@printf "%b" "$(LIN_COLOR)$(LIN_STRING) $(OBJ_COLOR)$(@)$(NO_COLOR)\n";
	@$(LINK.o) $^ $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_DIR)/%.d | $(OBJ_DIR) $(DEP_DIR)
	@printf "%b" "$(COM_COLOR)$(COM_STRING) $(OBJ_COLOR)$(@)$(NO_COLOR)\n";
	@$(PRECOMPILE)
	@$(COMPILE.cc) $< 
	@$(POSTCOMPILE)

$(OBJ_DIR):
	@mkdir -p $@

$(DEP_DIR):
	@mkdir -p $@

$(BIN_DIR):
	@mkdir -p $@

clean:
	@$(RM) -rv $(OBJ_DIR)
	@$(RM) -rv $(BIN_DIR)
	@$(RM) -rv $(DEP_DIR)	
	@$(RM) $(VER_HEADER)

.PRECIOUS: $(DEP_DIR)/%.d
$(DEP_DIR)/%.d: ;

-include $(DEP)

images: 
	podman/build.sh

#		 \


$(JPF_SCRIPT) = \
\#!/bin/sh\n\
	fpm \
		-s dir -t deb \
		-p /opt/$(JPF_DEB) \
		--name $(JPF_NAME) \
		--license Artistic-2.0 \
		--version $(JPF_VERSION)-$(JPF_RELEASE) \
		--architecture all \
		-d libc6 \
		-d libgcc-s1 \
		-d libstdc++6 \
		-d webfsd-jpf \
		--description \"John's Project Forecaster\" \
		--url \"https://github.com/j842/jpf\" \
		--maintainer \"John Enlow\" \
		/opt/$(JPF_EXE)=/usr/bin/$(JPF_NAME) \
		/opt/$(INP_DIR)=/opt/jpf/input/ \
#
$(JPF_SCRIPT): makefile
	@echo "$($(@))" | sed -e 's/^[ ]//' >$(@)
	chmod a+x $(JPF_SCRIPT)


# build debian package.
$(JPF_DEB): $(JPF_EXE) $(JPF_SCRIPT)
	$(RM) $(BIN_DIR)/$(JPF_NAME)*.deb
	podman run -it --rm -v $(ROOT_DIR):/opt/ jpf /bin/bash -c "/opt/$(JPF_SCRIPT)"

deb: $(JPF_DEB)

upload: deb
	podman run -it --rm -v $(ROOT_DIR):/opt \
		-e PACKAGECLOUD_TOKEN=${PACKAGECLOUD_TOKEN} \
		jpf package_cloud push j842/main/any/any /opt/$(JPF_DEB) 

# ------------------------------------------------
