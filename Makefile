C = gcc
CFLAGS = -Wall -g

TARGETS = monitor producer consumer

all: $(TARGETS)

monitor: monitor.o
	$(C) $(CFLAGS) -o $@ monitor.o

producer: producer.o
	$(C) $(CFLAGS) -o $@ producer.o

consumer: consumer.o
	$(C) $(CFLAGS) -o $@ consumer.o

clean:
	rm -rf *.o $(TARGETS) logfile
