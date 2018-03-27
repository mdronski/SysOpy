all: compile

compile:
	gcc main.c -o main
	gcc cpu_test_45th_fibonacci.c -o fibonacci
	gcc memory_test_1Gb.c -o alloc1Gb

tests:
	./main clearPass 1000 1000000
	./main clearPass 1000 100
	./main clearPass 5 1000000
