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
COPY     = cp
DEL      = rm -f
IPL10    = ./haribote/ipl10.bin
HARIBOTESYS = ./haribote/haribote.sys
HARIBOTEIMG = haribote.img

# デフォルト動作

default :
	$(MAKE) img

# ファイル生成規則

$(IPL10):
	$(MAKE) -C haribote

$(HARIBOTESYS):
	$(MAKE) -C haribote

haribote.img : $(IPL10) $(HARIBOTESYS) Makefile 
	mformat -f 1440 -C -B $(IPL10) -i haribote.img ::
	mcopy -i haribote.img $(HARIBOTESYS) ::
	# mcopy -i haribote.img nihongo.fnt ::
	# mcopy -i haribote.img haribote/ipl10.nas ::
	# mcopy -i haribote.img Makefile ::
	# mcopy -i haribote.img test.txt ::

nihongo:
	mcopy -D o -i haribote.img nihongo.fnt ::



# -i でイメージファイルを指定すると::でドライブにアクセスできるらしい

# コマンド

img :
	$(MAKE) haribote.img

run : 
	$(MAKE) -C haribote
	$(MAKE) haribote.img
	$(MAKE) -C apilib
	$(MAKE) -C apps
	$(MAKE) nihongo
	$(COPY) haribote.img ../../z_tools/qemu/fdimage0.bin
	$(MAKE) -C ../../z_tools/qemu

install :
	$(MAKE) img
	$(IMGTOL) w a: haribote.img

clean :
	-$(DEL) haribote.img
	$(MAKE) -C haribote clean
	$(MAKE) -C apilib clean
	$(MAKE) -C apps clean

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img

a_nasm.o : a_nasm.nas Makefile
	$(NASK) -g -f elf32 a_nasm.nas -o a_nasm.o -l a_nasm.lst

api%.o : api%.nas Makefile
	$(NASK) -g -f elf32 api%.nas -o api%.o -l api%.lst

apilib.a: $(OBJS_API) sprintf.o Makefile
	$(AR) r apilib.a $(OBJS_API) sprintf.o

%.hrb : %.o Makefile apilib.a Makefile api.ls apilib.h
	$(LD) -m elf_i386 -nostdlib -Tapi.ls $*.o apilib.a  -o $*.hrb -Map $*.map
