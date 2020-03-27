all: sender receiver

sender: sender.o
	g++ sender.o -o sender

receiver: recv.o
	g++ recv.o -o receiver

sender.o: sender.cpp
	g++ -c sender.cpp

recv.o: recv.cpp
	g++ -c recv.cpp

clean:
	rm *.o sender receiver

tar:
     tar cvf assignment1-Hammad_Qureshi_and_Adam_Laviguer.tar *.cpp *.h makefile 

