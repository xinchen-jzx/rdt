CC = g++
CFLAGS = -Wall -g
TARGET_CLIENT = client.out
TARGET_SERVER = server.out
SOURCES_CLIENT = rdt.cc utils.cc client.cc
SOURCES_SERVER = rdt.cc utils.cc server.cc
OBJECTS_CLIENT = $(SOURCES_CLIENT:.cc=.o)
OBJECTS_SERVER = $(SOURCES_SERVER:.cc=.o)

all: $(TARGET_CLIENT) $(TARGET_SERVER)

$(TARGET_CLIENT): $(OBJECTS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET_SERVER): $(OBJECTS_SERVER)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS_CLIENT) $(OBJECTS_SERVER) $(TARGET_CLIENT) $(TARGET_SERVER)
