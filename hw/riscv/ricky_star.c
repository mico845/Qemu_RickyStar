#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/sysbus.h"
#include "system/system.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/ricky_star.h"
#include "hw/riscv/boot.h"
#include "qemu/log-for-trace.h"
#include "hw/char/serial-mm.h"

static const MemMapEntry ricky_star_memmap[] = {
        [RICKY_STAR_MROM]  =        {        0x0,        0x8000  },
        [RICKY_STAR_UART]  =        { 0x10000000,         0x100  },

        [RICKY_STAR_DRAM]  =        { 0x80000000,        0x80000 },
};

static void ricky_star_cpu_create(MachineState *machine);
static void ricky_star_memory_create(MachineState *machine);
static void ricky_star_uart_create(MachineState *machine);


/* === Uart === */
static void ricky_star_uart_create(MachineState *machine)
{
    serial_mm_init(get_system_memory(),
                   ricky_star_memmap[RICKY_STAR_UART].base,
                   0,
                   NULL,
                   115200,
                   serial_hd(0),
                   DEVICE_NATIVE_ENDIAN);

}

/* === Memory === */
static void ricky_star_setup_rom_reset_vec(MachineState *machine, RISCVHartArrayState *harts,
                                           hwaddr start_addr,
                                           hwaddr rom_base, hwaddr rom_size,
                                           uint64_t kernel_entry,
                                           uint32_t fdt_load_addr)
{
    int i;
    uint32_t start_addr_hi32 = 0x00000000;

    if (!riscv_is_32bit(harts)) {
        start_addr_hi32 = start_addr >> 32;
    }
    /* reset vector */
    uint32_t reset_vec[10] = {
            0x00000297,                  /* 1:  auipc  t0, %pcrel_hi(fw_dyn) */ // 4
            0x02828613,                  /*     addi   a2, t0, %pcrel_lo(1b) */ // 8
            0xf1402573,                  /*     csrr   a0, mhartid  */          // 12
            0,                                                                  // 16
            0,                                                                  // 20
            0x00028067,                  /*     jr     t0 */                    // 24
            start_addr,                  /* start: .dword */                    // 28
            start_addr_hi32,                                                    // 32
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

    if (machine->kernel_filename)
    {
        RISCVBootInfo binfo = {
                .is_32bit = riscv_is_32bit(harts),
        };
        riscv_load_kernel(machine, &binfo, start_addr, true, NULL);
    }
}


static void ricky_star_memory_create(MachineState *machine)
{
    RickyStarState *s = RISCV_RICKY_STAR_MACHINE(machine);
    MemoryRegion *sys_mem = get_system_memory();
    MemoryRegion *dram = g_new(MemoryRegion, 1);
    MemoryRegion *mrom = g_new(MemoryRegion, 1);
    memory_region_init_ram(dram, NULL, "ricky_star.dram", ricky_star_memmap[RICKY_STAR_DRAM].size, &error_abort);
    memory_region_add_subregion(sys_mem, ricky_star_memmap[RICKY_STAR_DRAM].base, dram);

    memory_region_init_ram(mrom, NULL, "ricky_star.mrom", ricky_star_memmap[RICKY_STAR_MROM].size, &error_abort);
    memory_region_add_subregion(sys_mem, ricky_star_memmap[RICKY_STAR_MROM].base, mrom);


    ricky_star_setup_rom_reset_vec(machine, &s->cpu, ricky_star_memmap[RICKY_STAR_DRAM].base,
                                   ricky_star_memmap[RICKY_STAR_MROM].base,
                                   ricky_star_memmap[RICKY_STAR_MROM].size,
                                   0x0, 0x0);
}


/* === Cpu === */
static void ricky_star_cpu_create(MachineState *machine)
{
    RickyStarState *s = RISCV_RICKY_STAR_MACHINE(machine);

    qemu_log("Initializing CPU...\r\n");
    object_initialize_child(OBJECT(machine), "ricky_star.soc", &s->cpu, TYPE_RISCV_HART_ARRAY);
    object_property_set_str(OBJECT(&s->cpu), "cpu-type",
                            machine->cpu_type, &error_abort);
    object_property_set_int(OBJECT(&s->cpu), "hartid-base",
                            0, &error_abort);
    object_property_set_int(OBJECT(&s->cpu), "num-harts",
                            RICKY_STAR_CPUS_MAX, &error_abort);

    qemu_log("CPU initialized\r\n");
    sysbus_realize(SYS_BUS_DEVICE(&s->cpu), &error_abort);
    qemu_log("Sysbus realized\r\n");
}

/* === Machine === */
static void ricky_star_machine_init(MachineState *machine)
{
    qemu_log("ricky-star machine_init\r\n");
    ricky_star_cpu_create(machine);
    qemu_log("cpu created\r\n");
    ricky_star_memory_create(machine);
    qemu_log("memory created\r\n");
    ricky_star_uart_create(machine);
    qemu_log("uart created\r\n");
}

static void ricky_star_machine_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);
    mc->init = ricky_star_machine_init;
    mc->desc = "Ricky Star Board";
    mc->max_cpus = RICKY_STAR_CPUS_MAX;
    mc->default_cpu_type = TYPE_RISCV_CPU_BASE32;
}

static const TypeInfo ricky_star_machine_typeinfo = {
        .name       = TYPE_RISCV_RICKY_STAR_MACHINE,
        .parent     = TYPE_MACHINE,
        .class_init = ricky_star_machine_class_init,
        .instance_size = sizeof(RickyStarState),
};

static void ricky_star_machine_init_register_types(void)
{
    type_register_static(&ricky_star_machine_typeinfo);
}

type_init(ricky_star_machine_init_register_types)