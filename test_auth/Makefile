CC=cc
CFLAGS=-Wall -fPIC \
    -I../include \
    -I../include/freeDiameter \
    -I../build/include \
    -I/opt/homebrew/opt/gnutls/include

LDFLAGS= \
    -L../build \
    -L../build/libfdcore \
    -L../build/libfdproto \
    -L/opt/homebrew/opt/gnutls/lib \
    -lfdcore -lfdproto -lgnutls

all:
	$(CC) $(CFLAGS) test_auth.c -o test_auth.fdx -shared $(LDFLAGS)

clean:
	rm -f test_auth.fdx
