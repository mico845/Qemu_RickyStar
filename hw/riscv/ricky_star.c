#include "qemu/osdep.h"
#include "qemu/error-report.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/sysbus.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/ricky_star.h"
#include "hw/riscv/boot.h"
#include "qemu/log-for-trace.h"
#include "hw/riscv/numa.h"

static const MemMapEntry ricky_star_memmap[] = {
        [RICKY_STAR_MROM]  = {        0x0,        0x8000 },
        [RICKY_STAR_SRAM]  = {     0x8000,        0x8000 },
        [RICKY_STAR_UART0] = { 0x10000000,         0x100 },
        [RICKY_STAR_DRAM]  = { 0x80000000,           0x0 },
};

static void ricky_star_setup_rom_reset_vec(MachineState *machine,
                                           RISCVHartArrayState *harts, hwaddr start_addr,
                                           hwaddr rom_base, hwaddr rom_size,
                                           uint64_t kernel_entry, uint32_t fdt_load_addr)
{
    int i;
    uint32_t start_addr_hi32 = 0x00000000;

    if (!riscv_is_32bit(harts)) {
        start_addr_hi32 = start_addr >> 32;
    }
    /* reset vector */
    uint32_t reset_vec[10] = {
            0x00000297,                  /* 1:  auipc  t0, %pcrel_hi(fw_dyn) */
            0x02828613,                  /*     addi   a2, t0, %pcrel_lo(1b) */
            0xf1402573,                  /*     csrr   a0, mhartid  */
            0,
            0,
            0x00028067,                  /*     jr     t0 */
            start_addr,                  /* start: .dword */
            start_addr_hi32,
            fdt_load_addr,               /* fdt_laddr: .dword */
            0x00000000,
            /* fw_dyn: */
    };
    if (riscv_is_32bit(harts)) {
        reset_vec[3] = 0x0202a583;   /*     lw     a1, 32(t0) */
        reset_vec[4] = 0x0182a283;   /*     lw     t0, 24(t0) */
    } else {
        reset_vec[3] = 0x0202b583;   /*     ld     a1, 32(t0) */
        reset_vec[4] = 0x0182b283;   /*     ld     t0, 24(t0) */
    }

    /* copy in the reset vector in little_endian byte order */
    for (i = 0; i < ARRAY_SIZE(reset_vec); i++) {
        reset_vec[i] = cpu_to_le32(reset_vec[i]);
    }

    rom_add_blob_fixed_as("mrom.reset", reset_vec, sizeof(reset_vec),
                          rom_base, &address_space_memory);
}


/* === CPU === */
static void ricky_star_cpu_create(MachineState *machine)
{
    RISCVVirtState *s = RISCV_VIRT_MACHINE(machine);
    int i, base_hartid, hart_count;
    char *soc_name;

    if (RICKY_STAR_SOCKETS_MAX < riscv_socket_count(machine)) {
        error_report("number of sockets/nodes should be less than %d",
                     RICKY_STAR_SOCKETS_MAX);
        exit(1);
    }

    for (i = 0; i < riscv_socket_count(machine); i++) {
        if (!riscv_socket_check_hartids(machine, i)) {
            error_report("discontinuous hartids in socket%d", i);
            exit(1);
        }

        base_hartid = riscv_socket_first_hartid(machine, i);
        if (base_hartid < 0) {
            error_report("can't find hartid base for socket%d", i);
            exit(1);
        }

        hart_count = riscv_socket_hart_count(machine, i);
        if (hart_count < 0) {
            error_report("can't find hart count for socket%d", i);
            exit(1);
        }

        soc_name = g_strdup_printf("soc%d", i);
        object_initialize_child(OBJECT(machine), soc_name, &s->soc[i],
                                TYPE_RISCV_HART_ARRAY);
        g_free(soc_name);
        object_property_set_str(OBJECT(&s->soc[i]), "cpu-type",
                                machine->cpu_type, &error_abort);
        object_property_set_int(OBJECT(&s->soc[i]), "hartid-base",
                                base_hartid, &error_abort);
        object_property_set_int(OBJECT(&s->soc[i]), "num-harts",
                                hart_count, &error_abort);
        sysbus_realize(SYS_BUS_DEVICE(&s->soc[i]), &error_abort);
    }

}

static void ricky_star_memory_create(MachineState *machine)
{
    RISCVVirtState *s = RISCV_VIRT_MACHINE(machine);
    const MemMapEntry *memmap = ricky_star_memmap;
    MemoryRegion *system_memory = get_system_memory();
    MemoryRegion *main_mem = g_new(MemoryRegion, 1);
    MemoryRegion *sram_mem = g_new(MemoryRegion, 1);
    MemoryRegion *mask_rom = g_new(MemoryRegion, 1);

    memory_region_init_ram(main_mem, NULL, "riscv_quard_star_board.dram",
                           machine->ram_size, &error_fatal);
    memory_region_add_subregion(system_memory, memmap[RICKY_STAR_DRAM].base,
                                main_mem);

    memory_region_init_ram(sram_mem, NULL, "riscv_quard_star_board.sram",
                           memmap[RICKY_STAR_SRAM].size, &error_fatal);
    memory_region_add_subregion(system_memory, memmap[RICKY_STAR_SRAM].base,
                                sram_mem);

    memory_region_init_rom(mask_rom, NULL, "riscv_quard_star_board.mrom",
                           memmap[RICKY_STAR_MROM].size, &error_fatal);
    memory_region_add_subregion(system_memory, memmap[RICKY_STAR_MROM].base,
                                mask_rom);

    ricky_star_setup_rom_reset_vec(machine, &s->soc[0], memmap[RICKY_STAR_MROM].base,
                                   memmap[RICKY_STAR_MROM].base,
                                   memmap[RICKY_STAR_MROM].size,
                                   0x0, 0x0);

}

/* === Machine === */
static void ricky_star_machine_init(MachineState *machine)
{
    qemu_log("machine_init\n");
    ricky_star_cpu_create(machine);
    ricky_star_memory_create(machine);
}


static void ricky_star_machine_instance_init(Object *obj)
{
    qemu_log("machine_instance_init\n");
}


static void ricky_star_machine_class_init(ObjectClass *oc, void *data)
{
    qemu_log("machine_class_init\n");
    MachineClass *mc = MACHINE_CLASS(oc);
    mc->init = ricky_star_machine_init;
    mc->desc = "Ricky Star Board";
    mc->max_cpus = RICKY_STAR_CPUS_MAX;
    mc->default_cpu_type = TYPE_RISCV_CPU_BASE;
    mc->pci_allow_0_address = true;
    mc->possible_cpu_arch_ids = riscv_numa_possible_cpu_arch_ids;
    mc->cpu_index_to_instance_props = riscv_numa_cpu_index_to_props;
    mc->get_default_cpu_node_id = riscv_numa_get_default_cpu_node_id;
    mc->numa_mem_supported = true;
}

static const TypeInfo ricky_star_machine_typeinfo = {
        .name       = TYPE_RISCV_RICKY_STAR_MACHINE,
        .parent     = TYPE_MACHINE,
        .class_init = ricky_star_machine_class_init,
        .instance_init = ricky_star_machine_instance_init,
        .instance_size = sizeof(RISCVVirtState),
};

static void ricky_star_machine_init_register_types(void)
{
    type_register_static(&ricky_star_machine_typeinfo);
}

type_init(ricky_star_machine_init_register_types)