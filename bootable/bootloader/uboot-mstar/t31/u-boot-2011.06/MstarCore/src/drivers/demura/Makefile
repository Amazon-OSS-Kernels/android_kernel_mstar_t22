include $(TOPDIR)/config.mk
include $(TOPDIR)/../sboot/.config
include board_check.mk


COBJS		:=
UDV			:=
VENDOR		:= Mstar
LIB			:= $(obj)libdemura.a

CHIP_NAME	:= $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))

LOCAL_PATH	:= ./demura
HAL_PATH	:= ../../hal/$(CHIP_NAME)/demura

ifeq ($(URSA_TYPE),7622)
    URSA_DIR	:= M$(URSA_TYPE)
else ifeq ($(URSA_TYPE),7632)
    URSA_DIR	:= M$(URSA_TYPE)
else
URSA_DIR	:= U$(URSA_TYPE)
endif

###########  Base library  ###########
COBJS-y		+= ./base/dmalloc.o
COBJS-y		+= ./base/bits_stream_reader.o
COBJS-y		+= ./base/read_text.o
COBJS-y		+= ./base/ms_array.o
COBJS-y		+= ./base/ms_vector.o
COBJS-y		+= ./base/ms_utils.o
COBJS-y		+= ./base/crc_libs.o
CFLAGS		+= -I./
CPPFLAGS	+= -I./
CFLAGS		+= -I./base/
CPPFLAGS	+= -I./base/
CFLAGS		+= -I../../hal/$(CHIP_NAME)/demura/
CPPFLAGS	+= -I../../hal/$(CHIP_NAME)/demura/

COBJS-y		+= ./MsApiDemura.o
COBJS-y		+= ../../hal/$(CHIP_NAME)/demura/halRegOp.o

###########  Demura IP  ###########
ifeq ($(CONFIG_DEMURA_FCIC),y)
    DRV_PATH	:= ./fcic
    COBJS-y		+= ./fcic/Init_IP.o
    COBJS-y		+= ./fcic/MDrvFCIC.o
    COBJS-y		+= ../../hal/$(CHIP_NAME)/demura/halFCIC.o
    COBJS-y		:= $(COBJS-y)    # Unpack the macro right now
    CFLAGS		+= -I./mstar/
    CPPFLAGS	+= -I./mstar/
endif

ifeq ($(CONFIG_DEMURA_MSTAR),y)
    DRV_PATH	:= ./mstar/
    COBJS-y		+= ./mstar/Init_IP.o
    COBJS-y		+= ../../hal/$(CHIP_NAME)/demura/halDemura.o
    COBJS-y		+= ./mstar/MDrvDemura.o
    COBJS-y		+= ./mstar/convert_entry.o
    CFLAGS		+= -I./mstar/
    CPPFLAGS	+= -I./mstar/
 
    ###########  Panel Vendor  ###########
    ifneq ($(CONFIG_DEMURA_VENDOR_MSTAR),y)
        UDV			:= y
        ifeq ($(CONFIG_DEMURA_VENDOR_CSOT),y)
            VENDOR	:= csot
        else ifeq ($(CONFIG_DEMURA_VENDOR_AUO),y)
            VENDOR	:= auo
            COBJS-y	+= ./mstar/vendor/$(VENDOR)/parse_header.o
        else ifeq ($(CONFIG_DEMURA_VENDOR_INNOLUX),y)
            VENDOR	:= innolux
            COBJS-y	+= ./mstar/vendor/$(VENDOR)/decompress_8206.o
            COBJS-y	+= ./mstar/vendor/$(VENDOR)/parse_header.o
        else ifeq ($(CONFIG_DEMURA_VENDOR_LGD),y)
            VENDOR	:= lgd
            COBJS-y	+= ./mstar/vendor/$(VENDOR)/parse_fcic.o
        else ifeq ($(CONFIG_DEMURA_VENDOR_CHOT),y)
            VENDOR	:= chot
            COBJS-y	+= $(DRV_PATH)/vendor/$(VENDOR)/parse_header.o
        else ifeq ($(CONFIG_DEMURA_VENDOR_CSOT_CSOT),y)
            VENDOR	:= csot_csot
            COBJS-y	+= $(DRV_PATH)/vendor/$(VENDOR)/parse_header.o
        endif
    endif
    
    COBJS-$(UDV)	+= ./mstar/URSA/$(URSA_DIR)/demura.o
    COBJS-$(UDV)	+= ./mstar/URSA/$(URSA_DIR)/demura_common.o
    COBJS-$(UDV)	+= ./mstar/URSA/$(URSA_DIR)/demuraDLL.o
    COBJS-$(UDV)	+= ./mstar/vendor/$(VENDOR)/decoder_input.o
    CFLAGS			+= -I./mstar/URSA/
    CFLAGS			+= -I./mstar/URSA/$(URSA_DIR)/
    CFLAGS			+= -I./mstar/vendor/
    CPPFLAGS		+= -I./mstar/URSA/
    CPPFLAGS		+= -I./mstar/URSA/$(URSA_DIR)/
    CPPFLAGS		+= -I./mstar/vendor/
endif

###########  Pre Define  ###########
ifeq ($(URSA_TYPE),11)
CFLAGS		+= -DCONFIG_DEMURA_URSA11
CPPFLAGS	+= -DCONFIG_DEMURA_URSA11
else ifeq ($(URSA_TYPE),13)
CFLAGS		+= -DCONFIG_DEMURA_URSA13
CPPFLAGS	+= -DCONFIG_DEMURA_URSA13
else ifeq ($(URSA_TYPE),7632)
CFLAGS		+= -DCONFIG_DEMURA_M7632
CPPFLAGS	+= -DCONFIG_DEMURA_M7632
endif
CFLAGS	+= -DCONFIG_DEMURA_URSA_STRING=\"$(URSA_TYPE)\"
CFLAGS	+= -DCONFIG_DEMURA_VENDOR_STRING=\"$(VENDOR)\"
#CFLAGS += -include $(TOPDIR)/include/config.h

###########  Check Rule  ###########
CFLAGS	+= -Wextra
# Our function return type maybe int, double or float ..., While C default return type is "int" !
CFLAGS	+= -Werror=implicit-function-declaration

###########  # Complie rule  ###########


all:	$(LIB) $(AOBJS)

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))
#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
