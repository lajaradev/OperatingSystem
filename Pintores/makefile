CC=gcc
CFLAGS=-I.
INC=func.h sem.h
OBJECTS=pintar.o func.o sem.o
COMPLIB=*.gch
EXECUTABLE=pintar

pintar: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o $(EXECUTABLE)

pintar.o: pintar.c func.h
	$(CC) -c pintar.c $(CFLAGS)

func.o: func.c func.h
	$(CC) -c func.c  $(CFLAGS)

sem.o: sem.c $(INC)
	$(CC) -c sem.c  $(CFLAGS)

clean:
	rm -f $(OBJECTS) $(COMPLIB) 
	rm R*.dat
