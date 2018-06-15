test: hw.so
	./test.py

hw.so: generated.cpp
	g++ -shared -fPIC -o hw.so generated.cpp hello_world.cpp -lpython2.6 -lboost_python -I /usr/include/python2.6

generated.cpp:
	./generate.py
	
.PHONY: hw.so

clean:
	$(RM) exposed_decl.pypp.txt
	$(RM) generated.cpp
	$(RM) generated.cpp~
	$(RM) hello.pyc
	$(RM) hw.so
	$(RM) named_tuple.py
