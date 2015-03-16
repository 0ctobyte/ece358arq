CC := clang 
LD := gcc

PROGRAM := arq
C_SRCS := $(wildcard src/*.c)

OBJS := $(patsubst %.c,%.o,$(C_SRCS))

INCLUDE := -Iinclude

BASEFLAGS := 

WARNFLAGS   := -Weverything -Werror -Wno-missing-prototypes -Wno-unused-macros -Wno-padded -Wno-bad-function-cast -Wno-unused-parameter -Wno-format

CFLAGS := -std=c99 -g $(BASEFLAGS) $(WARNFLAGS) $(INCLUDE)
LDFLAGS := $(BASEFLAGS)

LIBS := 

all : $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

clean:
	$(RM) -f $(OBJS)
	$(RM) -f *.png *.dot *.pdf *.dat *.svg

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

distclean:
	$(RM) -f $(OBJS) $(PROGRAM)
	$(RM) -f *.png *.dot *.pdf *.dat *.svg

