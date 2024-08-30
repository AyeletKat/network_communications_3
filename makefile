
# CFLAGS = -Wall -g -Wextra

# .PHONY: all clean

# all: TCP_Sender TCP_Receiver RUDP_Receiver RUDP_Sender
# tcp: TCP_Sender TCP_Receiver
# rudp: RUDP_Receiver RUDP_Sender

# TCP_Sender : TCP_Sender.o
# 	gcc $(CFLAGS) -o sender TCP_Sender.o

# TCP_Receiver : TCP_Receiver.o
# 	gcc $(CFLAGS) -o receiver TCP_Receiver.o

# RUDP_Receiver: RUDP_Receiver.o 
# 	$(CC) $(CFLAGS) -o RUDP_Receiver RUDP_Receiver.o

# RUDP_Sender: RUDP_Sender.o 
# 	$(CC) $(CFLAGS) -o RUDP_Sender RUDP_Sender.o

# RUDP_Receiver.o: RUDP_Receiver.c RUDP_API.c
# 	$(CC) $(CFLAGS) -c RUDP_Receiver.c

# RUDP_Sender.o: RUDP_Sender.c RUDP_API.c
# 	$(CC) $(CFLAGS) -c RUDP_Sender.c

# TCP_Sender.o : TCP_Sender.c
# 	gcc $(CFLAGS) -c TCP_Sender.c

# TCP_Receiver.o : TCP_Receiver.c
# 	gcc $(CFLAGS) -c TCP_Receiver.c


# .PHONY: clean all

# clean:
# 	rm -f *.o sender receiver

CFLAGS = -Wall -g -Wextra

.PHONY: all clean tcp rudp

# Default target builds everything
all: tcp rudp

# Build only TCP programs
tcp: TCP_Sender TCP_Receiver

# Build only RUDP programs
rudp: RUDP_Sender RUDP_Receiver

# TCP targets
TCP_Sender: TCP_Sender.o
	$(CC) $(CFLAGS) -o TCP_Sender TCP_Sender.o

TCP_Receiver: TCP_Receiver.o
	$(CC) $(CFLAGS) -o TCP_Receiver TCP_Receiver.o

TCP_Sender.o: TCP_Sender.c
	$(CC) $(CFLAGS) -c TCP_Sender.c

TCP_Receiver.o: TCP_Receiver.c
	$(CC) $(CFLAGS) -c TCP_Receiver.c

# RUDP targets
RUDP_Receiver: RUDP_Receiver.o 
	$(CC) $(CFLAGS) -o RUDP_Receiver RUDP_Receiver.o

RUDP_Sender: RUDP_Sender.o 
	$(CC) $(CFLAGS) -o RUDP_Sender RUDP_Sender.o

RUDP_Receiver.o: RUDP_Receiver.c RUDP_API.c
	$(CC) $(CFLAGS) -c RUDP_Receiver.c

RUDP_Sender.o: RUDP_Sender.c RUDP_API.c
	$(CC) $(CFLAGS) -c RUDP_Sender.c

# Clean up generated files
clean:
	rm -f *.o TCP_Sender TCP_Receiver RUDP_Sender RUDP_Receiver

