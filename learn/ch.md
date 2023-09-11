# Architecture
## Memory layout

0        64K   640K        1M     3G         +640M          +256M  +12K           +4K               4G                         *4G              *4G -1M         -64K       MAX
+---------+-----+-----------+-----+------------+--------------+-----+--------------+-----------------+-----+----------------+---+----------------+---+-----------+----------+
| RAM-PIO | RAM | RSVD-EBDA | RAM | PCI Device | PCI-MMCONFIG | TSS | Identity map | TPM/APIC/IOAPIC | RAM | RAM-VIRTIO-MEM |...| PCI-SEG (PMEM) |   | PLAT MMIO | Reserved |
+---------+-----+-----------+-----+------------+--------------+-----+--------------+-----------------+-----+----------------+---+----------------+---+----------------------+
                                        0xE8000000     0xF8000000                                                                     0x3FFF00000000     0x3FFFFFFF0000

- PLAT MMIO:        DEV/CPU/MEM/GED HOTPLUG, 0x3ffffffef000, 3ffffffee000, 3ffffffed000
- PCI Device:       32bit PCI device, console, 0xe7f80000
- PCI SEG:          64bit PCI device, virtio-pmem, virtio-blk, 0x3ffe73380000
- IOAPIC_START:     0xFEC00000
- APIC_START:       0xFEE00000


