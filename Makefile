GCC=g++
CXX_FLAGS=--std=c++17 -pthread

clean: test
	rm $<

%: %.cc
	$(GCC) ${CXX_FLAGS} -o $@ $<
