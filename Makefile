#
# FIXME: SRC and LIBS if add c code or library
#
PROG=MRPserver
SRC = main.c input.c config.c log_db_pthread.c 

CC       = gcc
OBJ      = $(SRC:.c=.o)
LIBS    += -lpthread -lmysqlclient -L /usr/local/lib/mysql -I /usr/local/include/mysql
CFLAGS  += 
 
 
all: $(PROG)

$(PROG): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LIBS)
 
install:
	install -s -m 0755 $(PROG) /sbin/$(PROG)

clean:
	rm -f $(OBJ) $(PROG)
