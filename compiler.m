a=input('Enter instruction file name :','s');
ipt=fopen(a,'r');
b=input('Enter export file name :','s');
opt=fopen(b,'w');
mp=zeros(1,4);
mp(1)=input('Enter pitch of motor 1 in mm/rev :');
mp(2)=input('Enter pitch of motor 2 in mm/rev :');
mp(3)=input('Enter pitch of motor 3 in mm/rev :');
mp(4)=input('Enter pitch of motor 4 in mm/rev :');
%step	mod	spd/rep/rps	dir	time/delay
%4	1	16		1	15	
fmt=('%04f %01f %05f %01f %05f');
fmt2=('%04x%04x\n');

A=fscanf(ipt,fmt,[5 Inf]);
B=uint16(zeros(2,1024));
m=1;
sz=size(A);
sz=uint16(sz(2));
o=zeros(2,sz,'uint16');
mn=1;
ms=zeros(1,5);
ms(1)=0;
for m = 1:sz
		if A(3,m)>65534.9 && A(4,m)<0.1
			mn=mn+1;
			ms(mn)=m;
		end
		if A(2,sz)-1<0.01;
			B(1,m)=A(3,m)-1;
			B(2,m)=32768*A(4,m)+A(5,m);
	
		elseif A(2,sz)-2<0.01;
			B(1,m)=A(3,m)*A(5,m)*1792-1;
			B(2,m)=32768*A(4,m)+1/A(3,m)/1792*1000000;

		else
			B(1,m)=A(3,m)*A(5,m)*1792/mp(mn)-1;
			B(2,m)=32768*A(4,m)+1/A(3,m)*mp(mn)/1792*1000000;
		
		end


	
end
	ms(mn)=0;

	B(1,1023)=ms(1);
	B(2,1023)=ms(2);
	B(1,1024)=ms(3);
	B(2,1024)=ms(4);


dec2hex(B);
fprintf(opt,fmt2,B);
x=input('done, press ENTER to exit');



		
		end


	
end
ms(mn)=0;
n=1;
for m=2044:2048
	B(2,m)=ms(n);
	n=n+1;
end
dec2hex(B);
fprintf(opt,fmt2,B);
x=input('done, press ENTER to exit');



