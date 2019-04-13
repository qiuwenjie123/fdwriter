;使用nasm编译
	JMP	short entry
	nop
	DB		"QWJ     "		;厂商名    这里的空格是为了凑够字节数，下面同上     
	DW		512				;每扇区字节数
	DB		1				;每簇扇区数
	DW		1				;boot记录占用多少扇区
	DB		2				;共有多少FAT表
	DW		224				;根目录文件数最大值
	DW		2880			;扇区总数
	DB		0xf0			;介质描述符
	DW		9				;每fat扇区数
	DW		18				;每磁道扇区数
	DW		2				;磁头数
	DD		0				;隐藏扇区数
	DD		2880			;如果扇区总数是0，由这个值记录扇区数
	DB		0				;中断13的驱动器号
	DB      0				;未使用
	DB      0x29			;扩展引导标记
	DD		0				;卷序列号
	DB		"HARIBOTEOS "	  ;卷标
	DB		"FAT12   "	      ;	文件系统类型				
entry:
			; 引导代码、数据及其他填充字符
	times	510-($-$$) DB  0   ;使用0填充，使得正好512字节
	DW      0xaa55           ;结束标志