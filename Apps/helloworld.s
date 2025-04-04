	.section .text             //定义数据段名为.text
	.globl _start              //定义全局符号_start
	.type _start,@function     //_start为函数

_start:                        //函数入口

_init:
	li		t0,	0x100          //t0 = 0x100
	slli	t0,	t0, 20         //t0左移20位 t0 = 0x10000000
	li		t1,	'H'            //t1 = 'H' 字符的ASCII码值写入t1
	sb		t1, 0(t0)          //s是store写入的意思，b是byte，这里指的是写入t1
                               //的值到t0指向的地址，即为写入0x10000000这个寄存器
                               //这个寄存器正是uart0的发送data寄存器，此时串口会输出"H"
	li		t1,	'e'            //接下来都是重复内容
	sb		t1, 0(t0)
	li		t1,	'l'
	sb		t1, 0(t0)
	li		t1,	'l'
	sb		t1, 0(t0)
	li		t1,	'o'
	sb		t1, 0(t0)
	li		t1,	' '
	sb		t1, 0(t0)
	li		t1,	'Q'
	sb		t1, 0(t0)
	li		t1,	'u'
	sb		t1, 0(t0)
	li		t1,	'a'
	sb		t1, 0(t0)
	li		t1,	'r'
	sb		t1, 0(t0)
	li		t1,	'd'
	sb		t1, 0(t0)
	li		t1,	' '
	sb		t1, 0(t0)
	li		t1,	'S'
	sb		t1, 0(t0)
	li		t1,	't'
	sb		t1, 0(t0)
	li		t1,	'a'
	sb		t1, 0(t0)
	li		t1,	'r'
	sb		t1, 0(t0)
	li		t1,	' '
	sb		t1, 0(t0)
	li		t1,	'b'
	sb		t1, 0(t0)
	li		t1,	'o'
	sb		t1, 0(t0)
	li		t1,	'a'
	sb		t1, 0(t0)
	li		t1,	'r'
	sb		t1, 0(t0)
	li		t1,	'd'
	sb		t1, 0(t0)
	li		t1,	'!'
	sb		t1, 0(t0)
	li		t1,	'\n'
	sb		t1, 0(t0)          //到这里就会输出"Hello Quard Star board!"

_loop:
    j       _loop


    .end                       //汇编文件结束符号