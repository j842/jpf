#---------------------------------

JPF_VERSION := 0.0.10
JPF_RELEASE := 5
INPUT_VERSION := 6

#---------------------------------

SRC_DIR := src
OBJ_DIR := .obj
DEP_DIR := .dep
BIN_DIR := bin
INP_DIR := input

DEB_NAME := $(BIN_DIR)/jpf-$(JPF_VERSION)-$(JPF_RELEASE)-any.deb

EXE := $(BIN_DIR)/jpf
SRC := $(wildcard $(SRC_DIR)/*.cpp)
#HDR := $(wildcard $(SRC_DIR)/*.h)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEP := $(SRC:$(SRC_DIR)/%.cpp=$(DEP_DIR)/%.d)

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

.PHONY: all clean deb upload

all: $(EXE)

$(SRC_DIR)/version.h: makefile
	@echo "Generating $@"
	@echo "#define __JPF_VERSION \"${JPF_VERSION}\"" > $@
	@echo "#define __JPF_RELEASE \"${JPF_RELEASE}\"" >> $@
	@echo "#define __INPUT_VERSION (${INPUT_VERSION})" >> $@

$(EXE): $(OBJ) | $(BIN_DIR)
	$(LINK.o) $^ $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_DIR)/%.d | $(OBJ_DIR) $(DEP_DIR)
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

$(OBJ_DIR):
	mkdir -p $@

$(DEP_DIR):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(OBJ_DIR)
	@$(RM) -rv $(BIN_DIR)
	@$(RM) -rv $(DEP_DIR)
	@$(RM) $(DEB_NAME)
	

.PRECIOUS: $(DEP_DIR)/%.d
$(DEP_DIR)/%.d: ;

-include $(DEP)

# build debian package.
# dependencies are for the webfsd binary included ( https://packages.ubuntu.com/bionic/webfs )
$(DEB_NAME): $(EXE)
	@$(RM) $(DEB_NAME)
	fpm \
		-s dir -t deb \
		-p $(DEB_NAME) \
		--name jpf \
		--license Artistic-2.0 \
		--version $(JPF_VERSION)-$(JPF_RELEASE) \
		--architecture all \
		-d debconf \
		-d libc6 \
		-d libgnutls30 \
		-d lsb-base \
		-d ucf \
		-d libstdc++6 \
		--description "John's Project Forecaster" \
		--url "https://github.com/j842/jpf" \
		--maintainer "John Enlow" \
		$(EXE)=/usr/bin/jpf \
		contrib/webfsd-jpf=/usr/bin/webfsd-jpf
		$(INP_DIR)=/opt/jpf/input/

deb: $(DEB_NAME)


upload: deb
	package_cloud push j842/main/any/any $(DEB_NAME)
