test: json.cpp json.hpp test.cpp
	$(CXX) $(CANARY_ARGS) -O -std=c++11 json.cpp test.cpp -o test -fno-rtti -fno-exceptions

clean:
	if [ -e test ]; then rm test; fi

.PHONY: clean
