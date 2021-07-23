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
OBJS_API = api001.o api002.o api003.o api004.o api005.o api006.o api007.o api008.o api009.o api010.o

# デフォルト動作

default :
	$(MAKE) apilib.a

# ファイル生成規則


%.o : %.nas Makefile
	$(NASK) -g -f elf32 $*.nas -o $*.o -l $*.lst

../haribote/sprintf.o:
	$(MAKE) -C ../haribote sprintf.o

apilib.a: $(OBJS_API) ../haribote/sprintf.o Makefile
	$(AR) r apilib.a $(OBJS_API) ../haribote/sprintf.o

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
	-$(DEL) apilib.a

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img


