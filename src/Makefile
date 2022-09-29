BIN:=network.so
SRC:=$(wildcard *.c)
CFLAGS:=-fPIC -shared `net-snmp-config --cflags`
LDFLAGS:=-lnetsnmp -luci

.PHONY: all clean

$(BIN): $(SRC)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	$(RM) $(BIN)
