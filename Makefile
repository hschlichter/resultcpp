CXX = clang++
CXXFLAGS = -std=c++20 -O0 -g -Wall -Wextra -MMD -fPIC
LDFLAGS = 
INCLUDE = -I.
OUTDIR = ./out
LDINCLUDE =

result_tests: result_tests.cpp 
	$(CXX) $< -o $(OUTDIR)/result_tests $(CXXFLAGS) $(INCLUDE) $(LDINCLUDE) $(LDFLAGS) `pkg-config -cflags catch2 -libs catch2-with-main`

example: example.cpp
	$(CXX) $< -o $(OUTDIR)/example $(CXXFLAGS) $(INCLUDE) $(LDINCLUDE) $(LDFLAGS)

.PHONY: clean
clean:
	@rm -rf $(OUTDIR)

print-%  : ; @echo $* = $($*)
