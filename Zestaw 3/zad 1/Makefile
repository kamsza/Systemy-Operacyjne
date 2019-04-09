current_dir = $(shell pwd)

all: create compile test

compile:
	gcc -Wall -o main main.c
	
test:
	@echo "\n\n"
	@echo "======================= NFTW ======================="
	./main $(current_dir) nftw
	@echo "\n\n"
	@echo "======================= STAT ======================="
	./main $(current_dir) stat



create:
	mkdir -p dir1
	cd dir1
	mkdir -p dir1/tmp1 dir1/tmp2 dir1/tmp3 dir1/tmp4
	touch -d '2 February 2019 07:01' dir1/tmp1 dir1/tmp2
	touch -d '3 March 2019 10:22' dir1/tmp1/file1 dir1/tmp1/file2 dir1/tmp1/file3
	touch -d '7 March 2019 18:37' dir1/tmp2/file1 dir1/tmp2/file2 dir1/tmp2/file3
	touch dir1/tmp3/file1 dir1/tmp3/file2 dir1/tmp3/file3
	touch -d '2 March 2019 18:37' dir1/tmp4/file1 

clean:
	rm -rf dir1
	rm -f main nftw stat
