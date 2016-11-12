CXX=g++
OUT=./bin/pswdmgr

make:
	$(CXX) main.cpp myconio.cpp pswdmgr.cpp -lcryptolibrary -lscrypt -o $(OUT)

clean:
	rm $(OUT)
