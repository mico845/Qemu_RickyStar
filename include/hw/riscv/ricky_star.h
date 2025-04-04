#ifndef HW_RISCV_RICKY_STAR__H
#define HW_RISCV_RICKY_STAR__H

#include "hw/boards.h"
#include "hw/cpu/cluster.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/sysbus.h"
#include "hw/block/flash.h"
#include "qom/object.h"

#define RICKY_STAR_CPUS_MAX    1

/* === Machine === */

#define TYPE_RISCV_RICKY_STAR_MACHINE MACHINE_TYPE_NAME("ricky-star")
typedef struct RickyStarState RickyStarState;
#define RISCV_RICKY_STAR_MACHINE(obj) \
OBJECT_CHECK(RickyStarState, (obj), TYPE_RISCV_RICKY_STAR_MACHINE)


struct RickyStarState {
    /*< private >*/
    MachineState parent;

    /*< public >*/
    RISCVHartArrayState cpu;

};

enum {
    RICKY_STAR_MROM,
    RICKY_STAR_UART,
    RICKY_STAR_DRAM
};

#endif // HW_RISCV_RICKY_STAR__H
