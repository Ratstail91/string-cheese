CC=gcc
CFLAGS+=-g -Wall -Wextra -W -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable

OBJDIR=obj

SRC=$(wildcard *.c)
OBJ=$(addprefix $(OBJDIR)/,$(SRC:%.c=%.o))
OUT=a.out

all: $(OUT)

#output
$(OUT): $(OBJ)
	$(CC) -o $@ $(OBJ) $(CFLAGS)

#objects
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	@$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR):
	mkdir $(OBJDIR)

#cleanup
.PHONY: clean
clean:
	find . -type f -name '*.o' -exec rm -f -r -v {} \;
	find . -type f -name '*.out' -exec rm -f -r -v {} \;
	find . -empty -type d -delete
