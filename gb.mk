.PHONY: config build clear build-app

ROOT=$(shell pwd)
APPS_PATH=${ROOT}/Apps

run:
	${ROOT}/build/ricky_star.exe \
	-M ricky-star \
	-d in_asm \
	-kernel ${APPS_PATH}/output/helloworld.bin

build: clear
	make -j10
	mv build/qemu-system-riscv32.exe build/ricky_star.exe

config: clear
	./configure --target-list=riscv32-softmmu --enable-debug \
	--disable-install-blobs --disable-guest-agent --disable-sdl \
	--disable-curses --disable-werror --enable-tcg-interpreter

clear:
	clear

build-app:
	rm -rf ${APPS_PATH}/output
	bash ${APPS_PATH}/build.sh


