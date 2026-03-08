/* Copyright (C) 2021 Amazon.com Inc. or its affiliates.  All Rights Reserved. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <mbedtls/asn1.h>
#include <mbedtls/oid.h>
#include <mbedtls/x509_crt.h>

#include <amzn_secure_boot.h>
#include <amzn_crypto_impl.h>

#include "ufbl_debug.h" /* for dprintf */

#ifndef SHA256_DIGEST_LENGTH
#define SHA256_DIGEST_LENGTH 32
#endif
#define isascii(x) (((x) & ~0x7f) == 0)

__WEAK void amzn_target_sha256(const void *data, size_t n, void *digest)
{
	amzn_crypto_impl_sha256(data, n, digest);
}

static int isascii_s(const unsigned char *s, const size_t len)
{
	for (size_t idx = 0; idx < len; idx++) {
		if (!isascii(s[idx]))
			return 0;
	}

	return 1;
}

static int verify_org_unit(mbedtls_x509_crt *user_crt,  meta_data_handler handler)
{
	mbedtls_asn1_named_data *subject;
	int ret = -1;
	char *ou;

	for (subject = &user_crt->subject; subject; subject = subject->next) {
		const char *device_name = amzn_target_device_name();
		const mbedtls_asn1_buf oid = subject->oid;
		const unsigned int tag = subject->val.tag;
		const size_t device_name_len = strlen(device_name);
		const size_t ou_len = subject->val.len;

		if (MBEDTLS_OID_CMP(MBEDTLS_OID_AT_ORG_UNIT, &oid))
			continue;

		if (!MBEDTLS_ASN1_IS_STRING_TAG(tag))
			continue;

		if (!isascii_s((const unsigned char*)subject->val.p, ou_len))
			continue;

		ou = malloc((ou_len + 1) * sizeof(char));
		if (!ou) {
			dprintf(CRITICAL, "ou allocation failed\n");
			break;
		}

		ou[ou_len] = 0;

		memcpy(ou, subject->val.p, ou_len);
		dprintf(INFO, "%s: %d: ou=%s \n", __func__, __LINE__, ou);

		if (handler && handler(ou)) {
			dprintf(CRITICAL, "handler failed: %s\n", ou);
			ret = -1;
			free(ou);
			ou = NULL;
			break;
		}

		if (ou_len - 8 != device_name_len
			&& !strncmp(ou, "product:", 8)) {
			free(ou);
			ou = NULL;
			continue;
		}

		if (strncmp(ou + 8, device_name, device_name_len))
			ret = 0;

		free(ou);
		ou = NULL;
	}

	return ret;
}

static int verify_san_hash(mbedtls_x509_crt *user_crt, char *computed_hash)
{
	mbedtls_x509_sequence *san;
	size_t san_hash_len;
	mbedtls_mpi computed_hash_mpi;
	mbedtls_mpi san_hash_mpi;
	char *san_hash;
	int ret;

	mbedtls_mpi_init(&computed_hash_mpi);
	mbedtls_mpi_init(&san_hash_mpi);

	if (!(user_crt->ext_types & MBEDTLS_X509_EXT_SUBJECT_ALT_NAME))
		return -1;

	san = &user_crt->subject_alt_names;
	if (!san)
		return -1;

	san_hash_len = san->buf.len;
	if (san_hash_len != SHA256_DIGEST_LENGTH * 2 + 9) {
		dprintf(CRITICAL, "san is not the right size: %d\n", san_hash_len);
		return -1;
	}

	san_hash = malloc((san_hash_len + 1) * sizeof(char));
	if (!san_hash)
		return -1;

	memcpy(san_hash, san->buf.p, san_hash_len);
	san_hash[san_hash_len] = 0;

	if (strncmp(san_hash, "sha256://", 9)) {
		ret = -1;
		dprintf(CRITICAL, "SAN does not start with sha256://\n");
		goto free_hash;
	}

	ret = mbedtls_mpi_read_string(&san_hash_mpi, 16, &san_hash[9]);
	if (ret) {
		dprintf(CRITICAL, "Failed to parse san_hash\n");
		goto free_hash;
	}

	ret = mbedtls_mpi_read_binary(&computed_hash_mpi, (const unsigned char *)computed_hash, SHA256_DIGEST_LENGTH);
	if (ret) {
		dprintf(CRITICAL, "Failed to parse computed_hash\n");
		goto free_hash;
	}

	ret = mbedtls_mpi_cmp_mpi(&san_hash_mpi, &computed_hash_mpi);
	if (ret)
		goto free_hash;

	ret = 0;

	mbedtls_mpi_free(&san_hash_mpi);
	mbedtls_mpi_free(&computed_hash_mpi);

free_hash:
	free(san_hash);

	return ret;
}

int amzn_verify_image(int cert_type, char *computed_hash,
		      const unsigned char *usercert, meta_data_handler handler)
{
	const uint8_t *root_cert = NULL;
	mbedtls_x509_crt root_crt;
	mbedtls_x509_crt user_crt;
	size_t root_cert_len = 0;
	uint32_t flags = 0;
	int ret;

	/* Get the root CA certificate */
	root_cert = amzn_get_kernel_cert(cert_type, &root_cert_len);

	mbedtls_x509_crt_init(&root_crt);
	mbedtls_x509_crt_init(&user_crt);

	ret = mbedtls_x509_crt_parse(&root_crt, root_cert, root_cert_len);
	if (ret) {
		dprintf(CRITICAL, "Failed to parse root cert (%d)\n", ret);
		goto free_ctx;
	}

	/* Get user certificate */
	ret = mbedtls_x509_crt_parse(&user_crt, usercert, 2048);
	if (ret) {
		dprintf(CRITICAL, "Failed to parse user cert (%d)\n", ret);
		goto free_ctx;
	}

	/* Verify certificate */
	ret = mbedtls_x509_crt_verify(&user_crt, &root_crt, NULL, NULL, &flags, NULL, NULL);
	if (ret || flags) {
		dprintf(CRITICAL, "Failed to verify user cert flags=%d (%d)\n", flags, ret);
		goto free_ctx;
	}

	ret = verify_org_unit(&user_crt, handler);
	if (ret) {
		dprintf(CRITICAL, "verify_subject_node failed (%d)\n", ret);
		goto free_ctx;
	}

	ret = verify_san_hash(&user_crt, computed_hash);
	if (ret) {
		dprintf(CRITICAL, "verify_hash failed (%d)\n", ret);
	}

free_ctx:
	mbedtls_x509_crt_free(&user_crt);
	mbedtls_x509_crt_free(&root_crt);

	return ret;
}
