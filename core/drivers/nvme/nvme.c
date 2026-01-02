#include "nvme.h"

#include <stddef.h>

#include "alloc.h"
#include "core/header.h"
#include "io.h"
#include "pci.h"
#include "pci_utils.h"
#include "utils.h"

extern struct core_header g_core_header;

#define QEMU_NVME_VENDOR_ID 0x1b36
#define QEMU_NVME_DEVICE_ID 0x0010

static inline err_t ctrl_wait_ready(const struct nvme_ctrl* ctrl, uint8_t ready) {
  err_t err = SUCCESS;

  while ((read_mb32(ctrl->base_io + NVME_REG_CSTS) & NVME_CSTS_RDY) != ready);

  // Validate no fatal error occurred.
  uint32_t csts = read32(ctrl->base_io + NVME_REG_CSTS);
  CHECK((csts & NVME_CSTS_CFS) == 0);

cleanup:
  return err;
}

static err_t nvme_alloc_queue(struct nvme_ctrl* ctrl, struct nvme_queue* queue, uint16_t qid, uint16_t size) {
  err_t err = SUCCESS;

  queue->qid = qid;
  queue->q_depth = size;
  queue->sq_tail = 0;
  queue->cq_head = 0;
  // The completion queue phase initially begins at 1.
  queue->cq_phase = 1;

  queue->sq_doorbell = (ctrl->base_io + NVME_REG_DBS) + ((2 * qid) * ctrl->stride);
  queue->cq_doorbell = (ctrl->base_io + NVME_REG_DBS) + ((2 * qid + 1) * ctrl->stride);

  // The queue base address shall be memory page aligned (base on the value in `CC.MPS`).

  queue->cq_entries = core_malloc(ALIGN_UP(size * sizeof(struct nvme_completion), ctrl->page_size));
  CHECK(queue->cq_entries != NULL);

  queue->sq_entries = core_malloc(ALIGN_UP(size * sizeof(struct nvme_command), ctrl->page_size));
  CHECK(queue->sq_entries != NULL);

cleanup:
  return err;
}

err_t init_nvme_ctrl(struct nvme_ctrl* ctrl) {
  err_t err = SUCCESS;

  ctrl->pci_dev.addr.bus = g_core_header.disk_pci.addr.bus;
  ctrl->pci_dev.addr.device = g_core_header.disk_pci.addr.device;
  ctrl->pci_dev.addr.function = g_core_header.disk_pci.addr.function;

  CHECK_RETHROW(init_pci_dev(&ctrl->pci_dev));
  CHECK(ctrl->pci_dev.vendor_id == QEMU_NVME_VENDOR_ID && ctrl->pci_dev.device_id == QEMU_NVME_DEVICE_ID);

  for (size_t i = 0; i < PCI_BASE_ADDRESS_NUM; i++) {
    pci_write_32(&ctrl->pci_dev.addr, PCI_BASE_ADDRESS_0 + 4 * i, g_core_header.disk_pci.bars[i]);
  }

  CHECK(ctrl->pci_dev.header_type == PCI_HEADER_TYPE_NORMAL);

  uint16_t command = pci_read_16(&ctrl->pci_dev.addr, PCI_COMMAND);
  pci_write_16(&ctrl->pci_dev.addr, PCI_COMMAND, command | PCI_COMMAND_MEMORY);
  // TODO: Make the entire code above generic (since the virtio blk driver also shares it).

  // The base IO is stored in BAR0 (which may be concatenated with BAR1 in case of a 64bit address).
  struct pci_bar pci_bar = {0};
  CHECK_RETHROW(pci_get_bar(&ctrl->pci_dev, 0, &pci_bar));
  ctrl->base_io = (void*)pci_bar.addr;

  // Make sure the controller is reset.
  ctrl_wait_ready(ctrl, 0);

  uint64_t cap = read64(ctrl->base_io + NVME_REG_CAP);

  // The MQES is 0's based, i.e. the minimum value is 1, indicating two entries. We count it as 1's based.
  ctrl->max_queue_entries = NVME_CAP_MQES(cap) + 1;
  CHECK(ctrl->max_queue_entries > 2);

  // The stride is `2 ^ (2 + DSTRD)`.
  ctrl->stride = 1 << (2 + NVME_CAP_DSTRD(cap));

  // The minimum page size is `2 ^ (12 + MPSMIN)`. We just use the smallest possible page.
  ctrl->page_size = 1 << (12 + NVME_CAP_MPSMIN(cap));

  // The NVM command set must be supported.
  CHECK(NVME_CAP_CSS(cap) & NVME_CAP_CSS_NVM);

  uint16_t admin_queue_size = MIN(ctrl->max_queue_entries, NVME_ADMIN_QUEUE_DEPTH);
  // The minimum size of the admin queue is two entries.
  CHECK(admin_queue_size >= 2);

  CHECK_RETHROW(nvme_alloc_queue(ctrl, &ctrl->admin_queue, 0, admin_queue_size));

  // The AQA contains the admin completion and submission queue sizes.
  // The sizes are 0's based, so we subtract 1 since we store the sizes 1's based.
  uint32_t aqa = ((admin_queue_size - 1) << 16) | (admin_queue_size - 1);
  write32(ctrl->base_io + NVME_REG_AQA, aqa);

  // Set the admin completion and submission queues.
  write64(ctrl->base_io + NVME_REG_ACQ, (uintptr_t)ctrl->admin_queue.cq_entries);
  write64(ctrl->base_io + NVME_REG_ASQ, (uintptr_t)ctrl->admin_queue.sq_entries);

  mb();

  // Configurate the controller.
  uint32_t cc = 0;
  cc |= NVME_CC_CSS_NVM;                            // Use the NVM command set.
  cc |= NVME_CAP_MPSMIN(cap) << NVME_CC_MPS_SHIFT;  // Use the minimum possible page size.
  cc |= NVME_CC_AMS_RR;                             // Use the round robin arbitration mechanism.
  cc |= 6 << NVME_CC_IOSQES_SHIFT;                  // Submission entries are of size 64 (2^6).
  cc |= 4 << NVME_CC_IOCQES_SHIFT;                  // Completion entries are of size 16 (2^4).

  write_mb32(ctrl->base_io + NVME_REG_CC, cc);

  // Enable the controller.
  cc |= NVME_CC_EN;
  write_mb32(ctrl->base_io + NVME_REG_CC, cc);

  // Make sure the controller is ready for usage.
  CHECK_RETHROW(ctrl_wait_ready(ctrl, 1));

cleanup:
  return err;
}
