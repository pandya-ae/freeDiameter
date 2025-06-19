CFLAGS += -Wall -fPIC -I/usr/local/include/freeDiameter
LDFLAGS += -shared

all: test_auth.fdx

test_auth.fdx: test_auth.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f test_auth.fdx

