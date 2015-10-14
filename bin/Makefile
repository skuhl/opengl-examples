
.PHONY: all
all: cmake
	${MAKE} --quiet -C ..


.PHONY: cmake
cmake:
	cd .. && cmake .


%: cmake
	${MAKE} --quiet -C .. ${@}

Makefile:
	
