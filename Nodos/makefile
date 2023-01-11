CC=gcc
CFLAGS=-I. 
INC=func.h sem.h
OBJECTS=procesar.o func.o sem.o
COMPLIB=*.gch
EXECUTABLE=procesar

pintar: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o $(EXECUTABLE)

pintar.o: procesar.c func.h
	$(CC) -c pintar.c $(CFLAGS)

func.o: func.c func.h
	$(CC) -c func.c  $(CFLAGS)
	
sem.o: sem.c $(INC)
	$(CC) -c sem.c  $(CFLAGS)

clean:
	rm -f $(OBJECTS) $(COMPLIB) 
	rm jobsfile.txt 2>/dev/null
	rm jobscount.txt 2>/dev/null
