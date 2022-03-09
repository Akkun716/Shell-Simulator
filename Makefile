# Output binary name
bin=crash
lib=libshell.so

# Set the following to '0' to disable log messages:
LOGGER ?= 1

# Compiler/linker flags
CFLAGS += -g -Wall -fPIC -DLOGGER=$(LOGGER)
LDLIBS += -lm -lreadline
LDFLAGS += -L. -Wl,-rpath='$$ORIGIN'

# Source C files
src=history.c shell.c ui.c
obj=$(src:.c=.o)

all: $(bin) $(lib)

$(bin): $(obj)
	$(CC) $(CFLAGS) $(LDLIBS) $(LDFLAGS) $(obj) -o $@

$(lib): $(obj)
	$(CC) $(CFLAGS) $(LDLIBS) $(LDFLAGS) $(obj) -shared -o $@

shell.o: shell.c history.h logger.h ui.h
history.o: history.c history.h logger.h
ui.o: ui.h ui.c logger.h history.h

clean:
	rm -f $(bin) $(obj) $(lib) vgcore.*


# Tests --

test_repo=usf-cs326-sp22/P2-Tests

test: $(bin) $(lib) ./.testlib/run_tests ./tests
	@DEBUG="$(debug)" ./.testlib/run_tests $(run)

grade: ./.testlib/grade
	./.testlib/grade $(run)

testupdate: testclean test

testclean:
	rm -rf tests .testlib

./tests:
	rm -rf ./tests
	git clone https://github.com/$(test_repo) tests

./.testlib/run_tests:
	rm -rf ./.testlib
	git clone https://github.com/malensek/cowtest.git .testlib

./.testlib/grade: ./.testlib/run_tests
