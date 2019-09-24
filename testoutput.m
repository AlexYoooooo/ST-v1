a=input('Enter instruction file name :','s');

pkg load instrument-control
s1 = serial("/dev/ttyUSB0");
set(s1,'baudrate',600);
set(s1, 'bytesize',8);
set(s1,'parity','n');
set(s1,'stopbits',2);
set(s1,'timeout',10); 
srl_flush(s1); 
opt=uint8(srl_read(s1,10))



ipt=fopen(a,"r");
fmt=('%02x%02x%02x%02x');






A=uint8(zeros(4,1024));
A=fscanf(ipt,fmt,[4 Inf]);
sz=size(A);
sz=sz(2);
A=A';
su=sum(A);
su=sum(su)
srl_write(s1,uint8(0));
c=1;
for ln = 1:1024
	for col = 1:4
		A(ln,col)
		c=c+1
		srl_write(s1,uint8(A(ln,col)));
	end
end

	

c=1
opt=uint8(srl_read(s1,100))
s= size(opt)
s=s(2)
while s != 1
srl_write(s1,uint8(255));
c=c+1
opt=uint8(srl_read(s1,100))
s= size(opt)
s=s(2)

end


srl_write(s1,uint8(255));
x=input('done, press ENTER to exit');

opt=uint8(srl_read(s1,10))


		end
		opt=uint8(srl_read(s1,10));

	else
	break
	end
end
c=1
opt=uint8(srl_read(s1,100))
s= size(opt)
s=s(2)
while s != 1
srl_write(s1,uint8(255));
c=c+1
opt=uint8(srl_read(s1,100))
s= size(opt)
s=s(2)

end


srl_write(s1,uint8(255));
x=input('done, press ENTER to exit');

opt=uint8(srl_read(s1,10))

