all: writer reader

writer: writer.cpp lib.h ringbuffer.h
	g++ -std=c++17 -O2 -g writer.cpp -o writer -lpthread

reader: reader.cpp lib.h ringbuffer.h
	g++ -std=c++17 -O2 -g reader.cpp -o reader -lpthread

clean:
	rm -f writer reader
