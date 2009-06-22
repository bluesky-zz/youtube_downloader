
all:
	g++ -o main.o -c main.cpp
	g++ -o utils.o -c utils.cpp
	g++ -o bin/youtube_downloader -lcurl main.o utils.o

clean:
	rm *.o
	rm bin/youtube_downloader

install:
	cp bin/youtube_downloader /usr/local/bin/
