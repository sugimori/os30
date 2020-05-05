ipl.bin : ipl.nas Makefile
	nasm ipl.nas -o ipl.bin -l ipl.lst

helloos.img : ipl.bin Makefile
#	../../z_tools/edimg   imgin:../../z_tools/fdimg0at.tek \
#		wbinimg src:ipl.bin len:512 from:0 to:0   imgout:helloos.img
	mformat -f 1440 -C -B ipl.bin -i helloos.img ::

img :
	make -r helloos.img

run :
	make img
	cp helloos.img ../../z_tools/qemu/fdimage0.bin
	make -C ../../z_tools/qemu
