LDFLAGS = -pg
CFLAGS  = -pg -Wall -g3

ifdef OPTIMIZE
#CFLAGS = -g -Wall -O6 -funroll-loops -finline -march=i686
#CFLAGS = -g -Wall
#CFLAGS = -O6 -mcpu=i686 -march=i686 -fforce-addr -fomit-frame-pointer -funroll-loops -frerun-cse-after-loop -frerun-loop-opt -malign-functions=4
CFLAGS = -g -Wall -O6 -funroll-loops -finline-functions -march=i686 -fomit-frame-pointer
else
CFLAGS += -pg -g3 -Wall
endif

ifdef USE_EFENCE
LDFLAGS += -lefence
endif

.PHONY: ctags

OBJ := generate.o move_list.o move.o piece.o search.o evaluate.o board.o \
       play.o check.o move.o piece.o debug.o move_tree.o game.o str.o

SRC := $(OBJ:.o=.c)
DEP := $(OBJ:%.o=.%.d)

chess: $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@ 

ctags:
	ctags $(SRC)

.%.d: %.c
	@./depend.sh $*.c >$@

clean:
	rm -f $(OBJ) chess

distclean: clean
	rm -f $(DEP) tags

#
# Include dependencies
# The '-' in front of include skips it if the dep files do not exist
#
ifneq ($(findstring clean,$(MAKECMDGOALS)),clean)
-include $(DEP)
endif
