CXX = g++
CXXFLAGS = -fPIC -Wall -Wextra
LDFLAGS = -shared

all: main libhello.so

main: main.cpp
	$(CXX) main.cpp -o main -ldl

libhello.so: hello.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) hello.cpp -o libhello.so

clean:
	rm -f main libhello.
