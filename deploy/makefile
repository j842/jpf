ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
ROOT_DIR := $(abspath $(ROOT_DIR)/../)

include $(ROOT_DIR)/deploy/versions.makefile

JPF_DEB_NAME 	:= $(JPF_NAME)-$(JPF_VERSION)-$(JPF_RELEASE)-any.deb
JPF_DEB_PATH    := $(BIN_DIR)/$(JPF_DEB_NAME)
JPF_SCRIPT_NAME	:= $(JPF_NAME)_script.sh
JPF_SCRIPT_PATH := $(BIN_DIR)/$(JPF_SCRIPT_NAME)

.PHONY: images deb upload all

all: $(JPF_EXE) images deb

images: 
	$(ROOT_DIR)/deploy/podman/build.sh

$(JPF_SCRIPT_PATH) = \
\#!/bin/sh\n\
	fpm \
		-s dir -t deb \
		-p /opt/$(BIN_NAME)/$(JPF_DEB_NAME) \
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
		/opt/$(BIN_NAME)/$(JPF_NAME)=/usr/bin/$(JPF_NAME) \
#
$(JPF_SCRIPT_PATH): makefile
	@echo "$($(@))" | sed -e 's/^[ ]//' >$(@)
	chmod a+x $(JPF_SCRIPT_PATH)


# build debian package.
$(JPF_DEB_PATH): $(JPF_EXE) $(JPF_SCRIPT_PATH)
	$(RM) $(BIN_DIR)/$(JPF_NAME)*.deb
	podman run -it --rm -v $(ROOT_DIR):/opt/ jpf /bin/bash -c "/opt/$(BIN_NAME)/$(JPF_SCRIPT_NAME)"
	rm -f $(JPF_SCRIPT_PATH)

deb: $(JPF_DEB_PATH)

upload: deb
	podman run -it --rm -v $(ROOT_DIR):/opt \
		jpf package_cloud push j842/main/any/any /opt/$(BIN_NAME)/$(JPF_DEB_NAME) 

# ------------------------------------------------
