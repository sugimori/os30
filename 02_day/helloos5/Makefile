
# デフォルト動作

default :
	make img

# ファイル生成規則

ipl.bin : ipl.nas Makefile
	nasm ipl.nas -o ipl.bin -l ipl.lst

helloos.img : ipl.bin Makefile
	mformat -f 1440 -C -B ipl.bin -i helloos.img ::

# コマンド

asm :
	make -r ipl.bin

img :
	make -r helloos.img

run :
	make img
	cp helloos.img ../../z_tools/qemu/fdimage0.bin
	make -C ../../z_tools/qemu

install :
	make img
	../../z_tools/imgtol w a: helloos.img

clean :
	rm -f ipl.bin
	rm -f ipl.lst

src_only :
	make clean
	rm -f helloos.img
