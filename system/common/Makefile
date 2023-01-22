OBJS = asm_code.o asm_lang.o composite.o context.o data.o hash_table.o io.o location.o obj_code.o objects.o pointer.o primitive.o program.o scanner.o sentence.o src_code.o src_lang.o words.o

all: $(OBJS)
	$(AR) -r $(SDRFLOW_LIB) $(OBJS)

clean:
	rm -f $(OBJS) $(SDRFLOW_LIB)

install:

uninstall:
