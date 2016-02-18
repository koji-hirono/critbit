LIB=libcritbit.a
SRCS=critbit.c
OBJS=$(SRCS:.c=.o)

CFLAGS=-Wall -W
ARFLAGS=sr

all: $(LIB)

clean:
	rm -f $(OBJS) $(LIB) *~

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

$(OBJS): %.o : %.c
	$(CC) -c $(CFLAGS) -o $@ $<
