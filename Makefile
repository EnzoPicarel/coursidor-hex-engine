GSL_PATH ?= /net/ens/renault/save/gsl-2.6/install

CFLAGS = -std=c99 -Wall -Wextra -Werror=implicit-function-declaration -Werror=incompatible-pointer-types -fPIC -g3 -O0 -I$(GSL_PATH)/include 
LDFLAGS = -lm -lgsl -lgslcblas -ldl \
	-L$(GSL_PATH)/lib -L$(GSL_PATH)/lib64 \
	-Wl,--rpath=${GSL_PATH}/lib
COVERAGE_FLAGS = --coverage -fprofile-abs-path

SANITIZE_FLAGS = -fsanitize=address -fno-omit-frame-pointer


all: build test

build: server client client2
	@echo "Building all components..."

server: src/server.c gen_graph.o src/player.h src/move.h
	@echo "Compiling server..."
	@gcc $(CFLAGS) $< gen_graph.o -o $@ $(LDFLAGS)

player_fpic.o: src/astar_player.c src/astar_player.h src/player.h src/move.h src/graph.h
	@gcc $(CFLAGS) -I src -c $< -o $@

player2_fpic.o: src/random_player.c src/random_player.h src/player.h src/move.h src/graph.h
	@gcc $(CFLAGS) -I src -c $< -o $@	

gen_graph.o: src/gen_graph.c src/gen_graph.h src/graph.h
	@gcc $(CFLAGS) -c $< -o $@

gen_graph_cov.o: src/gen_graph.c src/gen_graph.h src/graph.h
	@gcc $(CFLAGS) $(COVERAGE_FLAGS) -c $< -o $@

player_cov.o: src/astar_player.c src/astar_player.h src/move.h src/graph.h
	@gcc $(CFLAGS) $(COVERAGE_FLAGS) -c $< -o $@

test_graph.o: test/test_graph.c src/gen_graph.h
	@gcc $(CFLAGS) -c $< -o $@

test_player.o: test/test_player.c src/astar_player.h src/gen_graph.h src/graph.h
	@gcc $(CFLAGS) -c $< -o $@

test_main.o: test/test_main.c
	@gcc $(CFLAGS) -c $< -o $@

client: player_fpic.o gen_graph.o
	@echo "Compiling client..."
	@gcc -I. -shared player_fpic.o gen_graph.o -o astar_player.so $(LDFLAGS)

client2: player2_fpic.o gen_graph.o
	@echo "Compiling client2..."
	@gcc -I. -shared player2_fpic.o gen_graph.o -o random_player.so $(LDFLAGS)


heldkarp_player.o: src/heldkarp_player.c src/heldkarp_player.h src/graph.h src/move.h 
	@gcc $(CFLAGS) -I src -c $< -o $@

client3: heldkarp_player.o gen_graph.o
	@echo "Compiling Client 3..."
	@gcc -I. -shared heldkarp_player.o gen_graph.o -o heldkarp_player.so $(LDFLAGS)

exec:
	@echo "Executing server..."
	@./install/server -m 4 -M 50 install/astar_player.so install/random_player.so

build_tests: test_main.o test_graph.o test_player.o gen_graph_cov.o player_cov.o
	@echo "Compiling tests..."
	@gcc $^ $(COVERAGE_FLAGS) -lgcov -o alltests $(LDFLAGS)
	@rm -f *.o

test: build_tests install
	@echo "Executing alltests"
	@./install/alltests;

install: build build_tests
	@echo "Installing..."
	@mkdir -p install
	@cp server *.so alltests install/

coverage: build_tests
	@echo "==> Running tests"
	@./alltests
	@echo "==> Generating coverage reports"
	@gcov gen_graph_cov.o player_cov.o > /dev/null
	@echo "==> Untested lines in gen_graph.c:"
	@grep "#####" gen_graph.c.gcov || echo "No untested lines"
	@echo "==> Untested lines in astar_player.c:"
	@grep "#####" astar_player.c.gcov || echo "No untested lines"

clean:
	@echo "Cleaning up..."
	@rm -f *~ src/*~ test/*~ 
	@rm -f *.o install/* alltests *.so server graph *.png *.dot
	@rm -rf install
	@find . \( -name "*.gcno" -o -name "*.gcda" -o -name "*.gcov" -o -name "*.gch" \) -exec rm -f {} \;

clang:
	@find . -name "*.[ch]" | xargs clang-format -i

rapport:
	pdflatex report/main.tex
	pdflatex report/main.tex
	mv main.* report/
	evince report/main.pdf &

.PHONY: all build server client client2 client3 install test clean exec build_tests coverage clang rapport
.PHONY: all build server client client2 install test clean graph exec build_tests coverage clang
