#
# Copyright 2021 Amazon.com, Inc. or its Affiliates. All rights reserved.
#
LOCAL_PATH := $(GET_LOCAL_DIR)

INCLUDES += \
	-I$(LOCAL_PATH)/src/library \
	-I$(LOCAL_PATH)/src/include \
	-I$(LOCAL_PATH)/src/configs


CFLAGS += \
	-O2 \
	-Wall \
	-Wextra \
	-D_FILE_OFFSET_BITS=64 \
	-DMBEDTLS_CONFIG_FILE='<config-ufbl.h>'

OBJS += \
	$(LOCAL_PATH)/src/library/aes.o \
	$(LOCAL_PATH)/src/library/aesni.o \
	$(LOCAL_PATH)/src/library/arc4.o \
	$(LOCAL_PATH)/src/library/aria.o \
	$(LOCAL_PATH)/src/library/asn1parse.o \
	$(LOCAL_PATH)/src/library/asn1write.o \
	$(LOCAL_PATH)/src/library/base64.o \
	$(LOCAL_PATH)/src/library/bignum.o \
	$(LOCAL_PATH)/src/library/blowfish.o \
	$(LOCAL_PATH)/src/library/camellia.o \
	$(LOCAL_PATH)/src/library/ccm.o \
	$(LOCAL_PATH)/src/library/chacha20.o \
	$(LOCAL_PATH)/src/library/chachapoly.o \
	$(LOCAL_PATH)/src/library/cipher.o \
	$(LOCAL_PATH)/src/library/cipher_wrap.o \
	$(LOCAL_PATH)/src/library/cmac.o \
	$(LOCAL_PATH)/src/library/ctr_drbg.o \
	$(LOCAL_PATH)/src/library/des.o \
	$(LOCAL_PATH)/src/library/dhm.o \
	$(LOCAL_PATH)/src/library/ecdh.o \
	$(LOCAL_PATH)/src/library/ecdsa.o \
	$(LOCAL_PATH)/src/library/ecjpake.o \
	$(LOCAL_PATH)/src/library/ecp.o \
	$(LOCAL_PATH)/src/library/ecp_curves.o \
	$(LOCAL_PATH)/src/library/entropy.o \
	$(LOCAL_PATH)/src/library/entropy_poll.o \
	$(LOCAL_PATH)/src/library/error.o \
	$(LOCAL_PATH)/src/library/gcm.o \
	$(LOCAL_PATH)/src/library/havege.o \
	$(LOCAL_PATH)/src/library/hkdf.o \
	$(LOCAL_PATH)/src/library/hmac_drbg.o \
	$(LOCAL_PATH)/src/library/md.o \
	$(LOCAL_PATH)/src/library/md2.o \
	$(LOCAL_PATH)/src/library/md4.o \
	$(LOCAL_PATH)/src/library/md5.o \
	$(LOCAL_PATH)/src/library/md_wrap.o \
	$(LOCAL_PATH)/src/library/memory_buffer_alloc.o \
	$(LOCAL_PATH)/src/library/nist_kw.o \
	$(LOCAL_PATH)/src/library/oid.o \
	$(LOCAL_PATH)/src/library/padlock.o \
	$(LOCAL_PATH)/src/library/pem.o \
	$(LOCAL_PATH)/src/library/pk.o \
	$(LOCAL_PATH)/src/library/pk_wrap.o \
	$(LOCAL_PATH)/src/library/pkcs12.o \
	$(LOCAL_PATH)/src/library/pkcs5.o \
	$(LOCAL_PATH)/src/library/pkparse.o \
	$(LOCAL_PATH)/src/library/pkwrite.o \
	$(LOCAL_PATH)/src/library/platform.o \
	$(LOCAL_PATH)/src/library/platform_util.o \
	$(LOCAL_PATH)/src/library/poly1305.o \
	$(LOCAL_PATH)/src/library/ripemd160.o \
	$(LOCAL_PATH)/src/library/rsa.o \
	$(LOCAL_PATH)/src/library/rsa_internal.o \
	$(LOCAL_PATH)/src/library/sha1.o \
	$(LOCAL_PATH)/src/library/sha256.o \
	$(LOCAL_PATH)/src/library/sha512.o \
	$(LOCAL_PATH)/src/library/threading.o \
	$(LOCAL_PATH)/src/library/timing.o \
	$(LOCAL_PATH)/src/library/version.o \
	$(LOCAL_PATH)/src/library/version_features.o \
	$(LOCAL_PATH)/src/library/xtea.o \
	$(LOCAL_PATH)/src/library/certs.o \
	$(LOCAL_PATH)/src/library/pkcs11.o \
	$(LOCAL_PATH)/src/library/x509.o \
	$(LOCAL_PATH)/src/library/x509_create.o \
	$(LOCAL_PATH)/src/library/x509_crl.o \
	$(LOCAL_PATH)/src/library/x509_crt.o \
	$(LOCAL_PATH)/src/library/x509_csr.o \
	$(LOCAL_PATH)/src/library/x509write_crt.o \
	$(LOCAL_PATH)/src/library/x509write_csr.o \
	$(LOCAL_PATH)/src/library/debug.o \
	$(LOCAL_PATH)/src/library/net_sockets.o \
	$(LOCAL_PATH)/src/library/ssl_cache.o \
	$(LOCAL_PATH)/src/library/ssl_ciphersuites.o \
	$(LOCAL_PATH)/src/library/ssl_cli.o \
	$(LOCAL_PATH)/src/library/ssl_cookie.o \
	$(LOCAL_PATH)/src/library/ssl_srv.o \
	$(LOCAL_PATH)/src/library/ssl_ticket.o \
	$(LOCAL_PATH)/src/library/ssl_tls.o


