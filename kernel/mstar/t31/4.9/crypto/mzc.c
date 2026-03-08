/*
 * Cryptographic API.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/crypto.h>
#include "mdrv_mzc_drv.h"

static int MZC_init(struct crypto_tfm *tfm)
{
	return 0;
}

static void MZC_exit(struct crypto_tfm *tfm)
{
}

static int MZC_compress(struct crypto_tfm *tfm, const u8 *src,
			    unsigned int slen, u8 *dst, unsigned int *dlen)
{
	int err;
#ifdef ENABLE_SINGLE_MODE
	err = MDrv_lenc_single_run(src, dst, dlen);
#else
	err = MDrv_lenc_cmdq_run(src, dst, dlen);
#endif
#ifdef CONFIG_MP_MZCCMDQ_HYBRID_HW
	tfm->is_mzc = 1;
#endif
	return err;
}

static int MZC_decompress(struct crypto_tfm *tfm, const u8 *src,
			      unsigned int slen, u8 *dst, unsigned int *dlen)
{
	int err;
#ifdef ENABLE_SINGLE_MODE
	err = MDrv_ldec_single_run(src,dst);
#else
	err = MDrv_ldec_cmdq_run(src,dst);
#endif
	if (err != PAGE_SIZE)
		return err;
	return 0;

}

static struct crypto_alg alg = {
	.cra_name		= "mzc",
	.cra_flags		= CRYPTO_ALG_TYPE_COMPRESS,
	.cra_ctxsize		= 0,
	.cra_module		= THIS_MODULE,
	.cra_init		= MZC_init,
	.cra_exit		= MZC_exit,
	.cra_u			= { .compress = {
	.coa_compress 		= MZC_compress,
	.coa_decompress  	= MZC_decompress } }
};

static int __init MZC_mod_init(void)
{
	return crypto_register_alg(&alg);
}

static void __exit MZC_mod_fini(void)
{
	crypto_unregister_alg(&alg);
}

module_init(MZC_mod_init);
module_exit(MZC_mod_fini);

MODULE_DESCRIPTION("MZC Compression Algorithm");
MODULE_ALIAS_CRYPTO("mzc");

