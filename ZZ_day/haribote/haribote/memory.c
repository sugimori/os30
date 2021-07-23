#include "bootpack.h"

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	// 386か、486か
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT; // AC-bit = 1
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if((eflg & EFLAGS_AC_BIT) != 0) {
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT; // AC-bit = 0
	io_store_eflags(eflg); // AC 0で書き込み

	if(flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; // キャッシュ禁止
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if(flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; // キャッシュ許可
		store_cr0(cr0);
	}

	return i;

}

unsigned int memtest_sub(unsigned int start, unsigned int end)
{
	unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
	for( i = start; i<= end; i += 0x1000) {
		p = (unsigned int *) (i + 0xffc);
		old = *p; 
		*p = pat0;
		*p ^= 0xffffffff;
		if( *p != pat1) {
not_memory:
			*p = old;
			break;
		}

		*p ^= 0xffffffff;
		if(*p != pat0) {
			goto not_memory;
		}
		*p = old;
	}
	return i;
}



void memman_init(struct MEMMAN *man)
{
	man->frees = 0;
	man->maxfrees = 0;
	man->lostsize = 0;
	man->losts = 0;
	return;
}

unsigned int memman_total(struct MEMMAN *man)
{
	unsigned int i, t =0;
	for(i=0;i<man->frees;i++) {
		t += man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
	unsigned int i, a;
	for(i=0; i < man->frees; i++) {
		if(man->free[i].size >= size) {
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if(man->free[i].size == 0) {
				man->frees--;
				for(;i < man->frees; i++) {		
					man->free[i] = man->free[i+1];	// 残りを前に詰める
				}
			}
			return a;
		}
	}
	return 0;
}


int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i, j;
	for(i=0; i < man->frees; i++) {
		if(man->free[i].addr > addr) {
			break;
		}
	}
	// free[i-1].addr < addr < free[i].addr
	if(i > 0) {
		if(man->free[i-1].addr + man->free[i-1].size == addr) {
			// 前のメモリに統合
			man->free[i-1].size += size;
			if( i < man->frees) {
				// 後ろもある
				if(addr + size == man->free[i].addr) {
					// 後ろも統合
					man->free[i-1].size += man->free[i].size;
					// man->free[i]の削除
					man->frees--;
					for(;i < man->frees; i++) {
						man->free[i] = man->free[i+1];
					}
				}
			}
			return 0;
		}
	}
	// 前とは統合できず
	if(i < man->frees) {
		// 後ろがある
		if(addr + size == man->free[i].addr) {
			// 後ろは統合可能
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;
		}
	}
	// 前も後ろも統合不可
	if (man->frees < MEMMAN_FREES) {
		// 後ろにずらす
		for(j = man->frees; j > i; j--) {
			man->free[j] = man->free[j-1];
		}
		man->frees++;
		if(man->maxfrees < man->frees) {
			man->maxfrees = man->frees;
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;
	}
	// 後ろにずらせなかった
	man->losts++;
	man->lostsize += size;
	return -1;

}


unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
    unsigned int a;
    size = (size + 0xfff) & 0xfffff000; // 切り上げ
    a = memman_alloc(man, size);
    return a;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
    int i;
    size = (size + 0xfff) & 0xfffff000;
    i = memman_free(man, addr, size);
    return i;
}