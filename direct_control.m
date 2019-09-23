pkg load instrument-control
s1 = serial("/dev/ttyUSB0");
set(s1,'baudrate',4800);
set(s1, 'bytesize',8);
set(s1,'parity','n');
set(s1,'stopbits',1);
set(s1,'timeout',10); 

srl_flush(s1); 

while 1
ipt=input('Enter command:' );
ipt=uint16(ipt);
disp(ipt);
srl_write(s1,ipt);
opt=srl_read(s1,10);
disp(opt)
endwhile

