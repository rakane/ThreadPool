compile:
	g++ example.cpp ThreadPool.h -pthread

clean:
	rm *.gch
	rm ./a.out
