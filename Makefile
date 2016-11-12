CXX=g++
OUT=./bin/pswdmgr

make:
	mkdir -p ./bin
	$(CXX) --std=c++11 main.cpp myconio.cpp pswdmgr.cpp -lcryptolibrary -lscrypt -o $(OUT)

clean:
	rm $(OUT)
