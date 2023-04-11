bgame:
	g++ bgame.cpp logging.c message.c -o bgame

clean:
	rm -rf bgame output_test.txt

test:
	make clean
	make bgame
	./bgame < input1.txt

