#---------------------------------

JPF_VERSION := 0.0.15
JPF_RELEASE := 11
INPUT_VERSION := 12
#---------------------------------

BIN_NAME:= build
INP_NAME:= input

SRC_DIR := $(ROOT_DIR)/src
OBJ_DIR := $(ROOT_DIR)/.obj
DEP_DIR := $(ROOT_DIR)/.dep
BIN_DIR := $(ROOT_DIR)/build
INP_DIR := $(ROOT_DIR)/$(INP_NAME)

JPF_NAME:= jpf
JPF_EXE := $(BIN_DIR)/$(JPF_NAME)