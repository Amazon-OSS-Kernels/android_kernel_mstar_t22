# Copyright (c) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved

#
# Common list of objects files we want to include
#
MBEDTLS_GPLV2_OBJECTS += \
        src/library/aes.o \
        src/library/aesni.o \
        src/library/arc4.o \
        src/library/aria.o \
        src/library/asn1parse.o \
        src/library/asn1write.o \
        src/library/base64.o \
        src/library/bignum.o \
        src/library/blowfish.o \
        src/library/camellia.o \
        src/library/ccm.o \
        src/library/chacha20.o \
        src/library/chachapoly.o \
        src/library/cipher.o \
        src/library/cipher_wrap.o \
        src/library/cmac.o \
        src/library/ctr_drbg.o \
        src/library/des.o \
        src/library/dhm.o \
        src/library/ecdh.o \
        src/library/ecdsa.o \
        src/library/ecjpake.o \
        src/library/ecp.o \
        src/library/ecp_curves.o \
        src/library/entropy.o \
        src/library/entropy_poll.o \
        src/library/error.o \
        src/library/gcm.o \
        src/library/havege.o \
        src/library/hkdf.o \
        src/library/hmac_drbg.o \
        src/library/md.o \
        src/library/md2.o \
        src/library/md4.o \
        src/library/md5.o \
        src/library/md_wrap.o \
        src/library/memory_buffer_alloc.o \
        src/library/nist_kw.o \
        src/library/oid.o \
        src/library/padlock.o \
        src/library/pem.o \
        src/library/pk.o \
        src/library/pk_wrap.o \
        src/library/pkcs12.o \
        src/library/pkcs5.o \
        src/library/pkparse.o \
        src/library/pkwrite.o \
        src/library/platform.o \
        src/library/platform_util.o \
        src/library/poly1305.o \
        src/library/ripemd160.o \
        src/library/rsa.o \
        src/library/rsa_internal.o \
        src/library/sha1.o \
        src/library/sha256.o \
        src/library/sha512.o \
        src/library/threading.o \
        src/library/timing.o \
        src/library/version.o \
        src/library/version_features.o \
        src/library/xtea.o \
        src/library/certs.o \
        src/library/pkcs11.o \
        src/library/x509.o \
        src/library/x509_create.o \
        src/library/x509_crl.o \
        src/library/x509_crt.o \
        src/library/x509_csr.o \
        src/library/x509write_crt.o \
        src/library/x509write_csr.o \
        src/library/debug.o \
        src/library/net_sockets.o \
        src/library/ssl_cache.o \
        src/library/ssl_ciphersuites.o \
        src/library/ssl_cli.o \
        src/library/ssl_cookie.o \
        src/library/ssl_srv.o \
        src/library/ssl_ticket.o \
        src/library/ssl_tls.o

MBEDTLS_GPLV2_CFLAGS += \
        -O2 \
        -Wall \
        -Wextra \
        -D_FILE_OFFSET_BITS=64 \
        -DMBEDTLS_CONFIG_FILE='<config-ufbl.h>'

MBEDTLS_GPLV2_HEADERS += \
    -I$(UFBL_PATH)/features/mbedtls-gplv2/src/configs \
    -I$(UFBL_PATH)/features/mbedtls-gplv2/src/include \
    -I$(UFBL_PATH)/features/mbedtls-gplv2/src/library

# Begin rules for kbuild U-Boot
ifeq (yes,$(strip $(UFBL_UBOOT_IS_KBUILD)))

ccflags-y += $(MBEDTLS_GPLV2_CFLAGS) $(MBEDTLS_GPLV2_HEADERS)
obj-y += $(MBEDTLS_GPLV2_OBJECTS)

else # default rules

include $(TOPDIR)/config.mk

MBEDTLS_GPLV2_LIB = $(obj)libmbedtls-gplv2.o

CFLAGS += \
    $(MBEDTLS_GPLV2_CFLAGS) \
    $(MBEDTLS_GPLV2_HEADERS)

COBJS  := $(MBEDTLS_GPLV2_OBJECTS)
SRCS   := $(COBJS:.o=.c)
OBJS   := $(addprefix $(obj),$(COBJS))

$(MBEDTLS_GPLV2_LIB): $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

endif # (yes,$(strip $(UFBL_UBOOT_IS_KBUILD)))
