OUTPUT_FORMAT("binary");

/*
 https://vanya.jp.net/os/haribote.html#hrb
・0x0000 : アプリ用データセグメントのサイズ
・0x0004 : 0x69726148 つまり”Hari”
・0x0008 : データセグメント内の予備領域の大きさ
・0x000c : ESP初期値
・0x0010 : データ部分（.dataセクション）のサイズ
・0x0014 : hrbファイル内にあるデータ部分の開始位置
・0x0018 : 0xe9000000
・0x001c : HariMainのアドレス-0x20
・0x0020 : malloc領域開始アドレス
*/
 
SECTIONS
{
    .head 0x0 : {
        LONG(128 * 1024)
        LONG(0x69726148)    /*  4 : シグネチャ "Hari" */
        LONG(0)
        LONG(0x0800)
        LONG(SIZEOF(.data))
        LONG(LOADADDR(.data))
        LONG(0xE9000000)
        LONG(HariMain - 0x20)
        LONG(24 * 1024)
    }
 
    .text : { *(.text) }
 
    .data 0x0800 : AT ( ADDR(.text) + SIZEOF(.text) ) {
        *(.data)
        *(.rodata)
        *(.bss*)
    }
 
    /DISCARD/ : { *(.eh_frame) }
}