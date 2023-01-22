OBJS = src_main.o
TARGET = sdfsrc
LDFLAGS = -s $(SDRFLOW_LIB)

all: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

install:
	cp -p $(TARGET) ../../$(BIN_DIR)

uninstall:
	rm -f ../../$(BIN_DIR)/$(TARGET)
