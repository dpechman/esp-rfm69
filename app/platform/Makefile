
#############################################################
# Required variables for each makefile
# Discard this section from all parent makefiles
# Expected variables (with automatic defaults):
#   CSRCS (all "C" files in the dir)
#   SUBDIRS (all subdirs with a Makefile)
#   GEN_LIBS - list of libs to be generated ()
#   GEN_IMAGES - list of images to be generated ()
#   COMPONENTS_xxx - a list of libs/objs in the form
#     subdir/lib to be extracted and rolled up into
#     a generated lib/image xxx.a ()
#
ifndef PDIR
GEN_LIBS = libplatform.a
endif

#############################################################
# Configuration i.e. compile options etc.
# Target specific stuff (defines etc.) goes in here!
# Generally values applying to a tree are captured in the
#   makefile at its root level - these are then overridden
#   for a subtree within the makefile rooted therein
#
#DEFINES +=
STD_CFLAGS=-std=gnu11 -Wimplicit

EXTRA_CCFLAGS += \
	-DPING_RATE=$(PING_RATE) -DDEFAULT_MQTT_IP=$(DEFAULT_MQTT_IP) \
	-DATMEGA_FLASH_MAX=$(ATMEGA_FLASH_MAX) -DRFM69_NODE_ID=$(RFM69_NODE_ID) \
	-DRFM69_ENCRYPT_KEY=$(RFM69_ENCRYPT_KEY) -DRFM69_NET_ID=$(RFM69_NET_ID) \
	-DRFM69_FREQ=$(RFM69_FREQ) -DRFM69_DEV_ID=$(RFM69_DEV_ID) \
	-DSTA_PASS="$(STA_PASS)" -DSTA_SSID="$(STA_SSID)" \
	-DVERSION="$(VERSION)" \
	-DMQTT_HOST_NAME="$(MQTT_HOST_NAME)" -DMQTT_ENABLE=$(MQTT_ENABLE) \
	-DMQTT_USE_IP=$(MQTT_USE_IP)

#############################################################
# Recursion Magic - Don't touch this!!
#
# Each subtree potentially has an include directory
#   corresponding to the common APIs applicable to modules
#   rooted at that subtree. Accordingly, the INCLUDE PATH
#   of a module can only contain the include directories up
#   its parent path, and not its siblings
#
# Required for each makefile to inherit from the parent
#

INCLUDES := $(INCLUDES) -I $(PDIR)include
INCLUDES += -I ./
INCLUDES += -I ../libc
INCLUDES += -I ../spiffs
INCLUDES += -I ../mqtt
INCLUDES += -I ../
PDIR := ../$(PDIR)
sinclude $(PDIR)Makefile
