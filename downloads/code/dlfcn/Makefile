all: fake_dlfcn testlib main

fake_dlfcn:
	g++ -std=c++17 -shared -fPIC fake_dlfcn.cpp -o libfake_dlfcn.so

testlib:
	g++ -std=c++17 -fPIC -shared -o libtestlib.so testlib.cpp

main:
	g++ -std=c++17 -o main main.cpp

