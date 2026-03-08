/*
 * FM(Shanghai fudan microelectronics co) SPI DataFlash support
 *
 * Copyright (C) 2019 MTK
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

/* M25Pxx-specific commands */
#define CMD_FM25_WREN       0x06    /* Write Enable */
#define CMD_FM25_WRDI       0x04    /* Write Disable */
#define CMD_FM25_RDSR       0x05    /* Read Status Register */
#define CMD_FM25_WRSR       0x01    /* Write Status Register */
#define CMD_FM25_READ       0x03    /* Read Data Bytes */
#define CMD_FM25_FAST_READ  0x0b    /* Read Data Bytes at Higher Speed */
#define CMD_FM25_PP         0x02    /* Page Program */
#define CMD_FM25_SE         0x20    /* Sector (4K) Erase */
#define CMD_FM25_BE         0xd8    /* Block (64K) Erase */
#define CMD_FM25_CE         0xc7    /* Chip Erase */
#define CMD_FM25_DP         0xb9    /* Deep Power-down */
#define CMD_FM25_RES        0xab    /* Release from DP, and Read Signature */

struct fm_spi_flash_params {
    uint16_t    id;
    /* Log2 of page size in power-of-two mode */
    uint8_t     l2_page_size;
    uint16_t    pages_per_sector;
    uint16_t    sectors_per_block;
    uint16_t    nr_blocks;
    const char  *name;
};

/* spi_flash needs to be first so upper layers can free() it */
struct fm_spi_flash {
    struct spi_flash flash;
    const struct fm_spi_flash_params *params;
};

static inline struct fm_spi_flash *
to_fm_spi_flash(struct spi_flash *flash)
{
    return container_of(flash, struct fm_spi_flash, flash);
}

static const struct fm_spi_flash_params fm_spi_flash_table[] = {
    {
        .id                 = 0x4014,
        .l2_page_size       = 8,
        .pages_per_sector   = 16,
        .sectors_per_block  = 16,
        .nr_blocks          = 16,
        .name               = "FM25Q08",
    },
};

static int fm_write(struct spi_flash *flash,
        u32 offset, size_t len, const void *buf)
{
    struct fm_spi_flash *stm = to_fm_spi_flash(flash);
    unsigned long page_addr;
    unsigned long byte_addr;
    unsigned long page_size;
    unsigned int page_shift;
    size_t chunk_len;
    size_t actual;
    int ret;
    u8 cmd[4];

    page_shift = stm->params->l2_page_size;
    page_size = (1 << page_shift);
    page_addr = offset / page_size;
    byte_addr = offset % page_size;

    ret = spi_claim_bus(flash->spi);
    if (ret) {
        debug("SF: Unable to claim SPI bus\n");
        return ret;
    }

    for (actual = 0; actual < len; actual += chunk_len) {
        chunk_len = min(len - actual, page_size - byte_addr);

        cmd[0] = CMD_FM25_PP;
        cmd[1] = page_addr >> (16 - page_shift);
        cmd[2] = page_addr << (page_shift - 8) | (byte_addr >> 8);
        cmd[3] = byte_addr;
        debug("PP: 0x%p => cmd = { 0x%02x 0x%02x%02x%02x } chunk_len = %d\n",
            buf + actual,
            cmd[0], cmd[1], cmd[2], cmd[3], chunk_len);

        ret = spi_flash_cmd(flash->spi, CMD_FM25_WREN, NULL, 0);
        if (ret < 0) {
            debug("SF: Enabling Write failed\n");
            goto out;
        }

        ret = spi_flash_cmd_write(flash->spi, cmd, 4,
                buf + actual, chunk_len);
        if (ret < 0) {
            debug("SF: fm Page Program failed\n");
            goto out;
        }

        ret = spi_flash_cmd_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
        if (ret)
            goto out;

        page_addr++;
        byte_addr = 0;
    }

    debug("SF: fm: Successfully programmed %u bytes @ 0x%x\n",
            len, offset);
    ret = 0;

out:
    spi_release_bus(flash->spi);
    return ret;
}

static int fm_erase(struct spi_flash *flash, u32 offset, size_t len)
{
    return spi_flash_cmd_erase(flash, CMD_FM25_SE, offset, len);
}

struct spi_flash *spi_flash_probe_fm(struct spi_slave *spi, u8 *idcode)
{
    const struct fm_spi_flash_params *params;
    unsigned page_size;
    struct fm_spi_flash *stm;
    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(fm_spi_flash_table); i++) {
        params = &fm_spi_flash_table[i];
        if (params->id == ((idcode[1] << 8) | idcode[2]))
            break;
    }

    if (i == ARRAY_SIZE(fm_spi_flash_table)) {
        debug("SF: Unsupported fm ID %02x%02x\n",
                idcode[1], idcode[2]);
        return NULL;
    }

    stm = malloc(sizeof(struct fm_spi_flash));
    if (!stm) {
        debug("SF: Failed to allocate memory\n");
        return NULL;
    }
    memset(stm, 0, sizeof(*stm));

    stm->params = params;
    stm->flash.spi = spi;
    stm->flash.name = params->name;

    /* Assuming power-of-two page size initially. */
    page_size = 1 << params->l2_page_size;

    stm->flash.write = fm_write;
    stm->flash.erase = fm_erase;
    stm->flash.read = spi_flash_cmd_read_fast;
    stm->flash.sector_size = (1 << stm->params->l2_page_size) *
        stm->params->pages_per_sector;
    stm->flash.size = page_size * params->pages_per_sector
                * params->sectors_per_block
                * params->nr_blocks;

    return &stm->flash;
}
