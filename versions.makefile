#---------------------------------

JPF_VERSION   := 0.0.18
JPF_RELEASE   := 18
INPUT_VERSION := 15
#---------------------------------

JPF_NAME:= jpf
BIN_NAME:= build
INP_NAME:= input

SRC_DIR := $(ROOT_DIR)/src
OBJ_DIR := $(ROOT_DIR)/.obj
DEP_DIR := $(ROOT_DIR)/.dep
BIN_DIR := $(ROOT_DIR)/build
INP_DIR := $(ROOT_DIR)/$(INP_NAME)

JPF_EXE      := $(BIN_DIR)/$(JPF_NAME)
JPF_FULL_VER := $(JPF_NAME) $(JPF_VERSION)-$(JPF_RELEASE)
