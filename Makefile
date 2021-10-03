CC			= gcc
CFLAGS			= -c -fPIC -Wall -Wextra -Wfatal-errors -Wno-unused-function
LDFLAGS      = -shared
SRCDIR 			= src
DEBUGOUTPUTDIR 		= build/debug
RELEASEOUTPUTDIR	= build/release
SOURCES			= src/endpoint.c
INCLUDES		= -I./src -Iinclude -I/usr/local/include/cd
_OBJECTS		= $(SOURCES:.c=.o)
LIBS			= -lcd -pthread
DEBUGOBJECTS 		= $(patsubst src/%,$(DEBUGOUTPUTDIR)/%,$(_OBJECTS))
RELEASEOBJECTS 		= $(patsubst src/%,$(RELEASEOUTPUTDIR)/%,$(_OBJECTS))
DEBUGTARGET		= build/debug/libendpoint.so
RELEASETARGET	= build/release/libendpoint.so

ldc := $(shell sudo ldconfig)
depcd := $(shell sudo ldconfig -p | grep libcd.so)

deps:
ifndef depcd
$(error "libcd $(depcd) is missing, please install libcd (https://github.com/dataandsignal/libcd)")
endif

debugprereqs:
		mkdir -p $(DEBUGOUTPUTDIR)

releaseprereqs:
		mkdir -p $(RELEASEOUTPUTDIR)

install-prereqs:
		sudo mkdir -p /usr/local/include/endpoint

debugall:	debugprereqs deps $(DEBUGTARGET)
releaseall:	releaseprereqs deps $(RELEASETARGET)

# additional flags
# CONFIG_DEBUG_LIST	- extensive debugging of list with external debugging
# 			functions
debug:		CFLAGS += -g -ggdb3 -O0
debug:		debugall

release:	CFLAGS +=
release: 	releaseall

examples-debug:		debugall install-debug
		cd examples && make examples-debug

examples-release:		releaseall install-release
		cd examples && make examples-release

examples:		examples-release

examples-clean:
		cd examples && make clean

$(DEBUGTARGET): $(DEBUGOBJECTS) 
	$(CC) $(DEBUGOBJECTS) $(LIBS) -o $@ $(LDFLAGS)

$(RELEASETARGET): $(RELEASEOBJECTS) 
	$(CC) $(RELEASEOBJECTS) $(LIBS) -o $@ $(LDFLAGS)

$(DEBUGOUTPUTDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

$(RELEASEOUTPUTDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

all: release

.DEFAULT_GOAL = release

install-headers: install-prereqs include/endpoint.h
	sudo cp include/* /usr/local/include/endpoint/

install-debug: $(DEBUGTARGET) install-headers
	sudo cp $(DEBUGTARGET) /lib/

install-release: $(RELEASETARGET) install-headers
	sudo cp $(RELEASETARGET) /lib/

install: install-release

uninstall:
	sudo rm /lib/libendpoint.so
	sudo rm -rf /usr/local/include/endpoint

clean:
	rm -rf $(DEBUGOBJECTS) $(DEBUGTARGET) $(RELEASEOBJECTS) $(RELEASETARGET)

clean-all: clean examples-clean
