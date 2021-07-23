#include "bootpack.h"

void file_raedfat(int *fat, unsigned char *img)
// ディスクイメージのFATの圧縮を解く
{
	int i, j = 0;
	for(i = 0; i < 2880; i += 2) {
		fat[i + 0] = (img[j + 0]		| img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4	| img[j + 2] << 4) & 0xfff;
		j += 3;
	}
	return;
}

void file_loadfile(int clustno, int size, char *buf, int *fat, char *img)
{
	int i;
	for(;;) {
		if(size <= 512) {
			for(i=0;i<size;i++) {
				buf[i] = img[clustno * 512 + i];
			}
			break;
		}
		// size > 512
		for(i=0;i<512;i++) {
			buf[i] = img[clustno * 512 + i];
		}
		size -= 512;
		buf += 512;
		clustno = fat[clustno];
	}
	return;
}

struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max)
{
	int i,j;
	char s[12];

	// ファイル名
	for(j = 0; j < 11; j++) {
		s[j] = ' ';
	}
	j = 0;
	for(i=0; name[i] != 0; i++) {
		if(j >= 11) return 0; // 見つからなかった
		if(name[i] == '.' && j <= 8) {
			j = 8;	// .がきたら拡張子なのでスキップ
		} else {
			s[j] = name[i];
			if('a' <= s[j] && s[j] <= 'z') {
				// 小文字だったら大文字に
				s[j] -= 0x20;
			}
			j++;
		}
	}
	// ファイルを探す
	for(i = 0; i < max;) {
		if(finfo[i].name[0] == 0x00) {
			break;
		}
		if((finfo[i].type & 0x18) == 0) {
			for(j=0;j<11;j++) {
				if(finfo[i].name[j] != s[j]) {
					goto next;	// x++;continue;でいいのでは？
				}
			}
			return finfo + i;// ファイルが見つかった
		}
next:
		i++;
	}
	return 0;
}