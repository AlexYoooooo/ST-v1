a=('Enter instruction file name :','s')

pkg load instrument-control
s1 = serial("/dev/ttyUSB0");
set(s1,'baudrate',4800);
set(s1, 'bytesize',8);
set(s1,'parity','n');
set(s1,'stopbits',1);
set(s1,'timeout',10); 
srl_flush(s1); 

ipt=fopen(a,"r");
fmt=('%02x %02x');



sz=size(A);
sz=uint16(sz(2));



A=uint8(fscanf(ipt,fmt,[4 Inf]));
A=A'
for ln = 1:sz
	for col = 1:4
		srl_write(s1,A(ln,col));
	endfor
endfor
x=input('done, press ENTER to exit');


