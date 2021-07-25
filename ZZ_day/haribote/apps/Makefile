TOOLPATH = ../../z_tools/
INCPATH  = ../../z_tools/haribote/

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
IPL10    = ../haribote/ipl10.bin
HARIBOTESYS = ../haribote/haribote.sys
HARIBOTEIMG = ../haribote.img
COPY     = cp
DEL      = rm -f
APP      = hello.hrb hello2.hrb a.hrb hello3.hrb hello4.hrb hello5.hrb winhelo.hrb winhelo2.hrb winhelo3.hrb star1.hrb stars.hrb stars2.hrb lines.hrb walk.hrb noodle.hrb beepdown.hrb color.hrb color2.hrb crack7.hrb sosu.hrb sosu2.hrb sosu3.hrb typeipl.hrb type.hrb iroha.hrb


# デフォルト動作

default :
	$(MAKE) all

all: $(APP)

# ファイル生成規則

../apilib/apilib.a:
	$(MAKE) -C ../apilib 

%.o: %.nas Makefile
	$(NASK) -g -f elf $*.nas -o $*.o -l $*.lst

%.o: %.c Makefile 
	$(CC) -march=i486 -m32 -nostdlib -c $*.c -o $*.o

%.hrb: %.o Makefile ../apilib/apilib.a Makefile ../apilib/api.ls ../apilib/apilib.h $(HARIBOTEIMG)
	$(LD) -m elf_i386 -nostdlib -T../apilib/api.ls $*.o ../apilib/apilib.a  -o $*.hrb -Map $*.map
	mcopy -D o -i $(HARIBOTEIMG) $*.hrb ::


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
	-$(DEL) *.hrb
	-$(DEL) *.map
	-$(DEL) bootpack.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys
	-$(DEL) hankaku.c
	-$(DEL) apilib.a

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img


