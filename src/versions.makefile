#---------------------------------
JPF_VERSION   := 0.0.23
JPF_RELEASE   := 007
INPUT_VERSION := 18
#---------------------------------

JPF_NAME:= jpf
BIN_NAME:= build
INP_NAME:= input

SRC_DIR := src
OBJ_DIR := .obj
DEP_DIR := .dep
BIN_DIR := build
INP_DIR := $(INP_NAME)

JPF_EXE      := $(BIN_DIR)/$(JPF_NAME)
JPF_FULL_VER := $(JPF_NAME) $(JPF_VERSION)-$(JPF_RELEASE)
