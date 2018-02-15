TARGET=ps_desktop

CPP=g++
LIBS=`pkg-config --cflags --libs pocketsphinx sphinxbase`
CFLAGS=-g -O3
OBJ=$(TARGET).o

all: $(TARGET).cpp
	$(CPP) -o $(TARGET) $(CFLAGS) $^ $(LIBS)

clean:
	rm -f *.o $(TARGET)

debug:
	gdb $(TARGET)

exec:
	./$(TARGET)
