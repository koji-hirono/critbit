SRCS:=$(shell ls -1 *.c)
OBJS=$(SRCS:.c=.o)
PROGS=$(patsubst %.c, %, $(SRCS))

LDADD=-lcritbit
CFLAGS=-Wall -W -I../ -L../

all: $(PROGS)

clean:
	rm -f $(OBJS) $(PROGS) *~

$(PROGS): % : %.o ../libcritbit.a
	$(CC) $(CFLAGS) -o $@ $< $(LDADD)

$(OBJS): %.o : %.c
	$(CC) -c $(CFLAGS) -o $@ $<
