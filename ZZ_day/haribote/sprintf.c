#include <stdarg.h>
 
//10進数からASCIIコードに変換
int dec2asc (char *str, int dec, int zero) {
    int len = 0, len_buf; //桁数
    int minus = 0;
    int buf[10];
    if( dec < 0) {
        minus = 1;
        dec *= -1;
    }
    while (1) { //10で割れた回数（つまり桁数）をlenに、各桁をbufに格納
        buf[len++] = dec % 10;
        if (dec < 10) break;
        dec /= 10;
    }
    if(len + minus > zero) {
        len_buf = len + minus;
    } else {
        len_buf = zero;
        while(zero > len + minus) {
            *(str++) = ' '; // スペースで埋める
            zero--;
        }
    }
    if(minus != 0) *(str++) = '-';
    while (len) {
        *(str++) = buf[--len] + 0x30;
    }
    return len_buf;
}
 
//16進数からASCIIコードに変換
int hex2asc (char *str, int dec) { //10で割れた回数（つまり桁数）をlenに、各桁をbufに格納
    int len = 0, len_buf; //桁数
    int buf[10];
    int minus = 0;
    if( dec < 0) {
        minus = 1;
        dec *= -1;
    }
    while (1) {
        buf[len++] = dec % 16;
        if (dec < 16) break;
        dec /= 16;
    }
    len_buf = len;
    while (len) {
        len --;
        *(str++) = (buf[len]<10)?(buf[len] + 0x30):(buf[len] - 9 + 0x60);
    }
    return len_buf;
}

//16進数からASCIIコードに変換
int hex2asclong (char *str, unsigned long dec) { //10で割れた回数（つまり桁数）をlenに、各桁をbufに格納
    int len = 0, len_buf; //桁数
    unsigned long  buf[32];
    int minus = 0;
    // if( dec < 0) {
    //     minus = 1;
    //     dec *= -1;
    // }
    while (1) {
        buf[len++] = dec % 16;
        if (dec < 16) break;
        dec /= 16;
    }
    len_buf = len;
    while (len) {
        len --;
        *(str++) = (buf[len]<10)?(buf[len] + 0x30):(buf[len] - 9 + 0x60);
    }
    return len_buf;
}

int sprintf (char *str, char *fmt, ...) {
    va_list list;
    int i, len=0;
    int count=0;
    va_start (list, fmt);
 
    while (*fmt) {
        int infmt = 0;
        if(*fmt=='%') {
            infmt = 1;
            count = 0;
            while(infmt) {
                fmt++;
                switch(*fmt){
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '0':
                        count = 10*count + *fmt - '0';
                        break;
                    case 'd':
                        len = dec2asc(str, va_arg (list, int), count);
                        infmt = 0;
                        break;
                    case 'x':
                        len = hex2asc(str, va_arg (list, int));
                        infmt = 0;
                        break;
                    case 'l':
                        len = hex2asclong(str, va_arg(list, unsigned long));
                        infmt = 0;
                        break;
                }
            }
            str += len; 
            fmt++;
        } else {
            *(str++) = *(fmt++);
        }   
    }
    *str = 0x00; //最後にNULLを追加
    va_end (list);

    return 0;
}