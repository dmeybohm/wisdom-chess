#CFLAGS = -g -Wall -O6 -funroll-loops -finline -march=i686
#CFLAGS = -g -Wall
CFLAGS = -O6 -mcpu=i686 -march=i686 -fforce-addr -fomit-frame-pointer -funroll-loops -frerun-cse-after-loop -frerun-loop-opt -malign-functions=4
#CFLAGS = -g -Wall -O6 -funroll-loops -finline-functions -march=i686 -fomit-frame-pointer
#LDFLAGS = -lefence

OBJ = generate.o move_list.o move.o piece.o search.o evaluate.o board.o \
      play.o check.o move.o piece.o debug.o move_tree.o

chess: $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@ 

include $(OBJ:.o=.d)

%.d: %.c
	./depend.sh $(CFLAGS) $<  >$@

clean:
	rm -f $(OBJ) chess
