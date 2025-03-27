#ifndef HW_RISCV_RICKY_STAR__H
#define HW_RISCV_RICKY_STAR__H

#include "hw/boards.h"
#include "hw/cpu/cluster.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/sysbus.h"
#include "hw/block/flash.h"
#include "qom/object.h"

#define RICKY_STAR_CPUS_MAX    8
#define RICKY_STAR_SOCKETS_MAX 8

/* === Machine === */

#define TYPE_RISCV_RICKY_STAR_MACHINE MACHINE_TYPE_NAME("ricky-star")
typedef struct RISCVVirtState RISCVVirtState;
#define RISCV_VIRT_MACHINE(obj) \
OBJECT_CHECK(RISCVVirtState, (obj), TYPE_RISCV_RICKY_STAR_MACHINE)


struct RISCVVirtState {
    /*< private >*/
    MachineState parent;

    /*< public >*/
    RISCVHartArrayState soc[RICKY_STAR_SOCKETS_MAX];

};

enum {
    RICKY_STAR_MROM,
    RICKY_STAR_SRAM,
    RICKY_STAR_UART0,
    RICKY_STAR_DRAM,
};

enum {
    RICKY_STAR_UART0_IRQ = 10,
};

#endif // HW_RISCV_RICKY_STAR__H
