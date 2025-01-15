all: writer reader

writer: writer.cpp lib.h
	g++ -std=c++17 -g writer.cpp -o writer -lpthread

reader: reader.cpp lib.h
	g++ -std=c++17 -g reader.cpp -o reader -lpthread

clean:
	rm -f writer reader
