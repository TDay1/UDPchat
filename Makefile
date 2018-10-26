COMPILER	 = gcc
TARGET       = udpchat
OBJECT       = main.o
SOURCE       = main.c

$(TARGET): $(OBJECT)
	$(COMPILER) -g -o $(TARGET) $(OBJECT) -pthread -lncurses

$(OBJECT) : $(SOURCE  )
	$(COMPILER) -g -c $(SOURCE) -o $(OBJECT)

 all : $(TARGET)

 clean :
	@rm $(TARGET) *.o
	@echo make dir cleaned

 run :
	@./udpchat