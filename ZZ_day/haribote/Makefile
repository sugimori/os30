TOOLPATH = ../../z_tools/
INCPATH  = ../../z_tools/haribote/

MAKE     = make -r
NASK     = nasm
#CC      = /usr/local/Cellar/x86_64-elf-gcc/9.3.0/bin/x86_64-elf-gcc
CC      = /usr/local/Cellar/x86_64-elf-gcc/10.2.0/bin/x86_64-elf-gcc
LD 		= /usr/local/Cellar/x86_64-elf-binutils/2.35/bin/x86_64-elf-ld
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
	$(MAKE) img

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

haribote.img : ipl10.bin haribote.sys Makefile hello.hrb hello2.hrb a.hrb hello3.hrb crack1.hrb crack2.hrb crack3.hrb crack4.hrb crack5.hrb crack6.hrb bug1.hrb bug2.hrb bug3.hrb hello4.hrb hello5.hrb winhelo.hrb winhelo2.hrb winhelo3.hrb
	mformat -f 1440 -C -B ipl10.bin -i haribote.img ::
	mcopy -i haribote.img haribote.sys ::
	mcopy -i haribote.img ipl10.nas ::
	mcopy -i haribote.img Makefile ::
	mcopy -i haribote.img test.txt ::
	mcopy -i haribote.img hello.hrb  ::
	mcopy -i haribote.img hello2.hrb  ::
	mcopy -i haribote.img a.hrb  ::
	mcopy -i haribote.img hello3.hrb  ::
	mcopy -i haribote.img crack1.hrb  ::
	mcopy -i haribote.img crack2.hrb  ::
	mcopy -i haribote.img crack3.hrb  ::
	mcopy -i haribote.img crack4.hrb  ::
	mcopy -i haribote.img crack5.hrb  ::
	mcopy -i haribote.img crack6.hrb  ::
	mcopy -i haribote.img bug1.hrb  ::
	mcopy -i haribote.img bug2.hrb  ::
	mcopy -i haribote.img bug3.hrb  ::
	mcopy -i haribote.img hello4.hrb  ::
	mcopy -i haribote.img hello5.hrb  ::
	mcopy -i haribote.img winhelo.hrb  ::
	mcopy -i haribote.img winhelo2.hrb  ::
	mcopy -i haribote.img winhelo3.hrb  ::

# -i でイメージファイルを指定すると::でドライブにアクセスできるらしい

# コマンド

img :
	$(MAKE) haribote.img

run : a.hrb
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

a_nasm.o : a_nasm.nas Makefile
	$(NASK) -g -f elf32 a_nasm.nas -o a_nasm.o -l a_nasm.lst

%.hrb : %.o Makefile a_nasm.o Makefile har.ld 
	$(LD) -m elf_i386 -nostdlib -Tapi.ls $*.o a_nasm.o -o $*.hrb -Map $*.map
