CC = clang++
CFLAGS = -std=c++20 -O0 -g -Wall -Wextra -MMD -fPIC
LDFLAGS = 
INCLUDE = -I.
OUTDIR = ./out
LDINCLUDE =

result_tests: result_tests.cpp 
	$(CC) $< -o $(OUTDIR)/result_tests $(CFLAGS) $(INCLUDE) $(LDINCLUDE) $(LDFLAGS) `pkg-config -cflags catch2 -libs catch2-with-main`

example0: example0.cpp
	$(CC) $< -o $(OUTDIR)/example0 $(CFLAGS) $(INCLUDE) $(LDINCLUDE) $(LDFLAGS)

.PHONY: clean
clean:
	@rm -rf $(OUTDIR)

print-%  : ; @echo $* = $($*)
