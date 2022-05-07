#---------------------------------

WEBJPF_VERSION := 0.0.2

#---------------------------------


ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BIN_DIR := bin

.PHONY: all clean webupload

all: webupload

clean:
	@$(RM) -rv $(BIN_DIR)

$(BIN_DIR):
	@mkdir -p $@


WEBJPF_NAME := webfsd-jpf
WEBJPF_SOURCE := $(WEBJPF_NAME)
WEBJPF_DEB := $(BIN_DIR)/$(WEBJPF_NAME)-$(WEBJPF_VERSION)-any.deb
WEBJPF_SCRIPT := $(BIN_DIR)/$(WEBJPF_NAME)_script.sh

# dependencies are for the webfsd binary included ( https://packages.ubuntu.com/bionic/webfs )
$(WEBJPF_SCRIPT) = \
\#!/bin/sh\n\
	fpm \
		-s dir -t deb \
		-p /opt/$(WEBJPF_DEB) \
		--name webfsd-jpf \
		--license Artistic-2.0 \
		--version $(WEBJPF_VERSION) \
		--architecture all \
		-d debconf \
		-d libc6 \
		-d libgnutls30 \
		-d lsb-base \
		-d ucf \
		--description \"Repackaged webfsd for jpf\" \
		--url \"https://github.com/j842/jpf\" \
		--maintainer \"John Enlow\" \
		/opt/$(WEBJPF_SOURCE)=/usr/bin/webfsd-jpf \
#

$(WEBJPF_SCRIPT): makefile $(BIN_DIR)
	@echo "$($(@))" | sed -e 's/^[ ]//' >$(@)
	chmod a+x $(WEBJPF_SCRIPT)


$(WEBJPF_DEB): $(WEBJPF_SCRIPT) 
	@$(RM) $(BIN_DIR)/$(WEBJPF_NAME)*.deb
	podman run -it --rm -v $(ROOT_DIR):/opt/ jpf /bin/bash -c "/opt/$(WEBJPF_SCRIPT)"

webupload: $(WEBJPF_DEB)
	podman run -it --rm -v $(ROOT_DIR):/opt \
		-e PACKAGECLOUD_TOKEN=${PACKAGECLOUD_TOKEN} \
		jpf package_cloud push j842/main/any/any /opt/$(WEBJPF_DEB) 

