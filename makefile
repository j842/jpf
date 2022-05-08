ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

include $(ROOT_DIR)/deploy/versions.makefile

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
#CXX=podman/podrun.sh g++

LD=$(CXX)
CXXFLAGS := -g -Wall -std=c++17 
LDFLAGS  := -g
LDLIBS   := -l:libboost_date_time.a -l:libcppunit.a
DEPFLAGS = -MT $@ -MD -MP -MF $(DEP_DIR)/$*.Td

PRECOMPILE =
POSTCOMPILE = mv -f $(DEP_DIR)/$*.Td $(DEP_DIR)/$*.d

COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) -c -o $@
LINK.o = $(LD) $(LDFLAGS) -o $@

.PHONY: all clean check

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

check: $(JPF_EXE)
	$(JPF_EXE) -t

deploy: $(JPF_EXE)
	gh auth login --with-token < ~/.github_token
	gh workflow run deploy_package.yml

# ------------------------------------------------
