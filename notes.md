(gdb) p *((struct FreeSegment*)(&KERNEL_END))
$3 = {size = 133030863, next_segment = 0x0}
(gdb) p &KERNEL_END
$4 = (unsigned long *) 0x1070a4
(gdb) x/2xw 0x1070a4
0x1070a4:       0x07ede3cf      0x00000000

