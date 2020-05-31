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

%.bin : %.nas Makefile
	$(NASK) $*.nas -o $*.bin -l $*.lst

naskfunc.o : naskfunc.nas Makefile
	$(NASK) -g -f elf naskfunc.nas -o naskfunc.o -l naskfunc.lst

hankaku.c : hankaku.txt
	makefont.rb > hankaku.c

%.o : %.c Makefile bootpack.h
	$(CC) -march=i486 -m32 -nostdlib -c $*.c -o $*.o
	
bootpack.hrb : bootpack.o naskfunc.o har.ld Makefile hankaku.o sprintf.o graphic.o dsctbl.o int.o fifo.o
	$(LD) -v -march=i486 -m elf_i386 -nostdlib -T har.ld bootpack.o naskfunc.o hankaku.o sprintf.o graphic.o dsctbl.o int.o fifo.o -o bootpack.hrb
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
	-$(DEL) hankaku.c

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img
