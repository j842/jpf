#---------------------------------

JPF_VERSION := 0.0.10
JPF_RELEASE := 1
INPUT_VERSION := 6

#---------------------------------

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
INP_DIR := input

DEB_NAME := $(BIN_DIR)/jpf-$(JPF_VERSION)-$(JPF_RELEASE)-any.deb

EXE := $(BIN_DIR)/jpf
SRC := $(wildcard $(SRC_DIR)/*.cpp)
HDR := $(wildcard $(SRC_DIR)/*.h)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# sudo apt-get install libboost-date-time-dev

CC=g++
CXXFLAGS := -g -Wall -std=c++17 
LDFLAGS  := -g 
LDLIBS   := -lboost_date_time

.PHONY: all clean deb upload

all: $(EXE)

$(SRC_DIR)/version.h: makefile
	@echo "Generating $@"
	@echo "#define __JPF_VERSION \"${JPF_VERSION}\"" > $@
	@echo "#define __INPUT_VERSION (${INPUT_VERSION})" >> $@

$(EXE): $(OBJ) | $(BIN_DIR) $(OTH_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HDR) | $(OBJ_DIR)
	$(CC) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(OBJ_DIR)
	@$(RM) -rv $(BIN_DIR)
	@$(RM) $(DEB_NAME)

-include $(OBJ:.o=.d)

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
		-d libboost-date-time1.71.0 \
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
