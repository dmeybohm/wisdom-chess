#CFLAGS = -g -Wall -O6 -funroll-loops -finline -march=i686
#CFLAGS = -g -Wall
#CFLAGS = -O6 -mcpu=i686 -march=i686 -fforce-addr -fomit-frame-pointer -funroll-loops -frerun-cse-after-loop -frerun-loop-opt -malign-functions=4
#CFLAGS = -g -Wall -O6 -funroll-loops -finline-functions -march=i686 -fomit-frame-pointer
CFLAGS = -g3 -Wall
#LDFLAGS = -lefence

.PHONY: ctags

SRC := generate.c move_list.o move.o piece.o search.o evaluate.o board.o \
      play.c check.o move.o piece.o debug.o move_tree.o
OBJ := $(SRC:.c=.o)

HEADERS := $(wildcard *.h)

chess: $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@ 

ctags:
	ctags *.c *.h

include $(OBJ:.o=.d)

%.d: %.c
	./depend.sh $(CFLAGS) $<  >$@

clean:
	rm -f $(OBJ) chess
