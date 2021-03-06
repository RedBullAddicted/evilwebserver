CC=gcc
CFLAGS=-pthread -ggdb -O -Wall -Wextra -m32
all:
	@mkdir build -p
	@echo Building libraries
	@make -C source/libraries/jsmn > build/build.log 2>&1
	@cd ../../
	@make -C source/libraries/objectivity > build/build.log 2>&1
	@cd ../../
	@echo Building EvilWebserver

	@$(CC) $(CFLAGS) source/base/webserver.c \
	 source/base/evilnetlib.c \
	  source/base/config.c \
	   source/libraries/jsmn/libjsmn.a \
	   source/libraries/objectivity/objectivity.a \
	   -o build/evilwebserver > build/build.log 2>&1
	@cp scripts build/ -r
	@cp config build/ -r
	@echo Done! Output written to build/build.log
clean:
	@echo Removing build folder
	@rm -rf build
	@echo Done cleaning up!