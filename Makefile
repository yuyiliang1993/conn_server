VERSION=stable-1.2
TARGET=conn_server
OBJS=conn_server.o mylog.o myredis.o\
mysock.o cJSON.o public.o *.h ./hiredis/libhiredis.a
CC=gcc
DEBUG= #-g -O0
CFLAGS= $(DEBUG) -Wall -D__DEBUG_OUT__ -DVERSION=\"$(VERSION)\"
LDFLAGS=-lhiredis
INCLUDE=-I./hiredis
INSNAME=$(TARGET)-$(VERSION).tar.gz
all:$(TARGET)
$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@
%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@
./PHONY:clean install dist
install:
	mkdir -p /var/log/
	mkdir -p /usr/local/bin/
	cp $(TARGET) /usr/local/bin/ -f
dist:
	cd ..;tar czvf $(INSNAME) conn_server
	mv ../$(INSNAME) ./
clean:
	rm -rf *.o $(TARGET) $(INSNAME)
