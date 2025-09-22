#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "error.h"

/**
 * Recursively scans ACPI table (beginning at the UEFI system table which points to the RSDP) to locate all the required
 * ACPI tables, and initializes their glboal variables (e.g. `g_facs`, `g_fadt`, etc).
 *
 * On a successful call, all the global variables representing ACPI tables should be initialized to non-null values.
 */
err_t find_acpi_tables(void);

// ---- ACPI Tables -----

struct acpi_rsdp {
  char signature[8];
  uint8_t checksum;
  char oemid[6];
  uint8_t revision;
  uint32_t rsdt_addr;

  // These fields are only available if `revision >= 2`.
  uint32_t length;
  uint64_t xsdt_addr;
  uint8_t extended_checksum;
  uint8_t rsvd[3];
} __attribute__((packed));

struct acpi_table_header {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemid[6];
  char oem_table_id[8];
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
} __attribute__((packed));

struct acpi_rxsdt {
  struct acpi_table_header hdr;
  uint8_t ptr_bytes[];
} __attribute__((packed));

struct acpi_gas {
  uint8_t address_space_id;
  uint8_t register_bit_width;
  uint8_t register_bit_offset;
  uint8_t access_size;
  uint64_t address;
} __attribute__((packed));

struct acpi_fadt {
  struct acpi_table_header hdr;
  uint32_t firmware_ctrl;
  uint32_t dsdt;
  uint8_t int_model;
  uint8_t preferred_pm_profile;
  uint16_t sci_int;
  uint32_t smi_cmd;
  uint8_t acpi_enable;
  uint8_t acpi_disable;
  uint8_t s4bios_req;
  uint8_t pstate_cnt;
  uint32_t pm1a_evt_blk;
  uint32_t pm1b_evt_blk;
  uint32_t pm1a_cnt_blk;
  uint32_t pm1b_cnt_blk;
  uint32_t pm2_cnt_blk;
  uint32_t pm_tmr_blk;
  uint32_t gpe0_blk;
  uint32_t gpe1_blk;
  uint8_t pm1_evt_len;
  uint8_t pm1_cnt_len;
  uint8_t pm2_cnt_len;
  uint8_t pm_tmr_len;
  uint8_t gpe0_blk_len;
  uint8_t gpe1_blk_len;
  uint8_t gpe1_base;
  uint8_t cst_cnt;
  uint16_t p_lvl2_lat;
  uint16_t p_lvl3_lat;
  uint16_t flush_size;
  uint16_t flush_stride;
  uint8_t duty_offset;
  uint8_t duty_width;
  uint8_t day_alrm;
  uint8_t mon_alrm;
  uint8_t century;
  uint16_t iapc_boot_arch;
  uint8_t rsvd;
  uint32_t flags;
  struct acpi_gas reset_reg;
  uint8_t reset_value;
  uint16_t arm_boot_arch;
  uint8_t fadt_minor_verison;
  uint64_t x_firmware_ctrl;
  uint64_t x_dsdt;
  struct acpi_gas x_pm1a_evt_blk;
  struct acpi_gas x_pm1b_evt_blk;
  struct acpi_gas x_pm1a_cnt_blk;
  struct acpi_gas x_pm1b_cnt_blk;
  struct acpi_gas x_pm2_cnt_blk;
  struct acpi_gas x_pm_tmr_blk;
  struct acpi_gas x_gpe0_blk;
  struct acpi_gas x_gpe1_blk;
  struct acpi_gas sleep_control_reg;
  struct acpi_gas sleep_status_reg;
  uint64_t hypervisor_vendor_identity;
} __attribute__((packed));

#define ACPI_GLOBAL_LOCK_OWNED (1 << 1)
struct acpi_facs {
  char signature[4];
  uint32_t length;
  uint32_t hardware_signature;
  uint32_t firmware_waking_vector;
  uint32_t global_lock;
  uint32_t flags;
  uint64_t x_firmware_waking_vector;
  uint8_t version;
  char rsvd0[3];
  uint32_t ospm_flags;
  char rsvd1[24];
};

#define DSDT_AML_SIZE(dsdt) ((dsdt).hdr.length - sizeof(dsdt))

struct acpi_dsdt {
  struct acpi_table_header hdr;
  uint8_t definition_block[];
} __attribute__((packed));

struct acpi_ssdt {
  struct acpi_table_header hdr;
  uint8_t definition_block[];
} __attribute__((packed));

// These tables are resolved after a successful call to `find_acpi_tables`.
extern struct acpi_rsdp* g_rsdp;
extern struct acpi_rxsdt* g_rxsdt;
extern struct acpi_fadt* g_fadt;
extern struct acpi_facs* g_facs;
extern struct acpi_dsdt* g_dsdt;
