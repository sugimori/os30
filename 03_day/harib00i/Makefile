TOOLPATH = ../../z_tools/
INCPATH  = ../../z_tools/haribote/

MAKE     = make -r
NASK     = nasm
CC      = /usr/local/Cellar/x86_64-elf-gcc/9.3.0/bin/x86_64-elf-gcc
LD 		= /usr/local/Cellar/x86_64-elf-binutils/2.34/bin/x86_64-elf-ld
GAS2NASK = $(TOOLPATH)gas2nask -a
OBJ2BIM  = $(TOOLPATH)obj2bim
BIM2HRB  = $(TOOLPATH)bim2hrb
RULEFILE = $(TOOLPATH)haribote/haribote.rul
EDIMG    = $(TOOLPATH)edimg
IMGTOL   = $(TOOLPATH)imgtol
COPY     = cp
DEL      = rm -f

# デフォルト動作

default :
	$(MAKE) img

# ファイル生成規則

ipl10.bin : ipl10.nas Makefile
	$(NASK) ipl10.nas -o ipl10.bin -l ipl10.lst

asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas -o asmhead.bin -l asmhead.lst

# bootpack.gas : bootpack.c Makefile
# 	$(CC1) -o bootpack.gas bootpack.c

# bootpack.nas : bootpack.gas Makefile
# 	$(GAS2NASK) bootpack.gas bootpack.nas

# bootpack.obj : bootpack.nas Makefile
# 	$(NASK) bootpack.nas bootpack.obj bootpack.lst

# bootpack.bim : bootpack.obj Makefile
# 	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
# 		bootpack.obj
# 3MB+64KB=3136KB

# bootpack.hrb : bootpack.bim Makefile
# 	$(BIM2HRB) bootpack.bim bootpack.hrb 0

bootpack.o : bootpack.c
	$(CC) -march=i486 -m32 -nostdlib -c bootpack.c -o bootpack.o

bootpack.hrb : bootpack.o har.ld Makefile
	$(LD) -v -march=i486 -m elf_i386 -nostdlib -T har.ld bootpack.o -o bootpack.hrb
#	$(CC) -v -march=i486 -m32 -nostdlib -T har.ld bootpack.o -o bootpack.hrb


haribote.sys : asmhead.bin bootpack.hrb Makefile
	cat asmhead.bin bootpack.hrb > haribote.sys

haribote.img : ipl10.bin haribote.sys Makefile
	mformat -f 1440 -C -B ipl10.bin -i haribote.img ::
	mcopy -i haribote.img haribote.sys ::

# コマンド

img :
	$(MAKE) haribote.img

run :
	$(MAKE) img
	$(COPY) haribote.img ../../z_tools/qemu/fdimage0.bin
	$(MAKE) -C ../../z_tools/qemu

install :
	$(MAKE) img
	$(IMGTOL) w a: haribote.img

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) *.o
	-$(DEL) bootpack.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img
