
HELLOWORLD_FOLDER=E:/Project/qemu/Qemu_RickyStar/Apps
CROSS_PREFIX=E:/Environment/msys2/mingw64/bin/riscv64-unknown-elf

if [ ! -d "$HELLOWORLD_FOLDER/output" ]; then
  mkdir $HELLOWORLD_FOLDER/output
fi

# 编译汇编文件helloworld.s到obj文件
$CROSS_PREFIX-gcc -march=rv32ima -mabi=ilp32 -x assembler-with-cpp -c helloworld.s -o $HELLOWORLD_FOLDER/output/helloworld.o

echo "> helloworld.o"

# 使用链接脚本链接obj文件生成elf可执行文件
$CROSS_PREFIX-gcc -march=rv32ima -mabi=ilp32 -nostartfiles -T memory.lds -Wl,-Map=$HELLOWORLD_FOLDER/output/helloworld.map -Wl,--gc-sections $HELLOWORLD_FOLDER/output/helloworld.o -o $HELLOWORLD_FOLDER/output/helloworld.elf

echo "> helloworld.elf"

# 使用gnu工具生成原始的程序bin文件
$CROSS_PREFIX-objcopy -O binary -S $HELLOWORLD_FOLDER/output/helloworld.elf $HELLOWORLD_FOLDER/output/helloworld.bin

echo "> helloworld.bin"

# 使用gnu工具生成反汇编文件，方便调试分析（当然我们这个代码太简单，不是很需要）
$CROSS_PREFIX-objdump --source --demangle --disassemble --reloc --wide $HELLOWORLD_FOLDER/output/helloworld.elf > $HELLOWORLD_FOLDER/output/helloworld.lst

echo "> helloworld.lst"
