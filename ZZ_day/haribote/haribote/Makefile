TOOLPATH = ../../../z_tools/
INCPATH  = ../../../z_tools/haribote/

MAKE     = make -r
NASK     = nasm
#CC      = /usr/local/Cellar/x86_64-elf-gcc/9.3.0/bin/x86_64-elf-gcc
CC       = /usr/local/Cellar/x86_64-elf-gcc/10.2.0/bin/x86_64-elf-gcc
LD 		   = /usr/local/Cellar/x86_64-elf-binutils/2.35/bin/x86_64-elf-ld
AR       = /usr/local/Cellar/x86_64-elf-binutils/2.35/bin/x86_64-elf-ar
RANLIB   = /usr/local/Cellar/x86_64-elf-binutils/2.35/bin/x86_64-elf-ranlib
GAS2NASK = $(TOOLPATH)gas2nask -a
OBJ2BIM  = $(TOOLPATH)obj2bim
BIM2HRB  = $(TOOLPATH)bim2hrb
RULEFILE = $(TOOLPATH)haribote/haribote.rul
EDIMG    = $(TOOLPATH)edimg
IMGTOL   = $(TOOLPATH)imgtol
COPY     = cp
DEL      = rm -f
OBJS_BOOTPACK = bootpack.o naskfunc.o hankaku.o sprintf.o graphic.o dsctbl.o int.o fifo.o keyboard.o mouse.o memory.o sheet.o timer.o mtask.o strcmp.o window.o console.o file.o

# デフォルト動作

default :
	$(MAKE) haribote.sys
	$(MAKE) ipl10.bin

# ファイル生成規則

%.bin : %.nas Makefile
	$(NASK) $*.nas -o $*.bin -l $*.lst

naskfunc.o : naskfunc.nas Makefile
	$(NASK) -g -f elf naskfunc.nas -o naskfunc.o -l naskfunc.lst

%.o : %.nas Makefile
	$(NASK) -g -f elf $*.nas -o $*.o -l $*.lst

hankaku.c : hankaku.txt
	./makefont.rb > hankaku.c

%.o : %.c Makefile bootpack.h
	$(CC) -march=i486 -m32 -nostdlib -c $*.c -o $*.o
	
bootpack.hrb : $(OBJS_BOOTPACK) Makefile har.ld 
	$(LD) -v -march=i486 -m elf_i386 -nostdlib -T har.ld $(OBJS_BOOTPACK) -o bootpack.hrb -Map bootpack.map
#	$(CC) -v -march=i486 -m32 -nostdlib -T har.ld bootpack.o -o bootpack.hrb

haribote.sys : asmhead.bin bootpack.hrb Makefile
	cat asmhead.bin bootpack.hrb > haribote.sys

# コマンド
clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) *.o
	-$(DEL) *.map
	-$(DEL) bootpack.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys
	-$(DEL) hankaku.c

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img

