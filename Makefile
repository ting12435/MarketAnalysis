
all: bin
clean: clean-bin

bin:
	make -C libs/util
	make -C libs/BSDR
	make -C libs/MI_Index
	make -C util

clean-bin:
	make -C libs/util clean
	make -C libs/BSDR clean
	make -C libs/MI_Index clean
	make -C util clean
	make -C run/bsdr_analysis clean
