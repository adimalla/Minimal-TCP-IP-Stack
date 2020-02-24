

#! /bin/bash

CC := gcc
CFLAGS := -Wall -Wextra
BUILD := build_temp
OBJECT_DIR := objs
BIN := bin

TARGET := mqtt_publish

APPOBJECTS := main.o publisher_methods.o iot_client.o
APPINCLUDES := headers.h error_codes.h iot_client.h

APIOBJECT := mqtt_client.o
APIINCLUDES := mqtt_client.h mqtt_configs.h

default:
	rm -rf $(OBJECT_DIR) $(BIN)
	mkdir $(OBJECT_DIR) $(BIN)
	cp -r ../../API/inc/*.h ../../API/src/*.c $(PWD)
	$(MAKE) -C $(PWD) $(TARGET)  
	mv $(TARGET) $(BIN)
	mv -f *.o $(OBJECT_DIR)
	rm mqtt_client.* mqtt_configs.h

.PHONY:	$(TARGET)

$(TARGET):	$(APPOBJECTS) $(APIOBJECT)
	$(CC) -o $(TARGET) $(APPOBJECTS) $(APIOBJECT)


main.o:	main.c $(APPINCLUDES)
	$(CC) -c main.c $(CFLAGS)

publisher_methods.o:	publisher_methods.c $(APPINCLUDES)
	$(CC) -c publisher_methods.c $(CFLAGS)

iot_client.o:	iot_client.c $(APPINCLUDES)
	$(CC) -c iot_client.c $(CFLAGS)

$(APIOBJECT):	mqtt_client.c $(APIINCLUDES)
	$(CC) -c mqtt_client.c $(CFLAGS)


.PHONY: clean

clean:
	rm -rf $(OBJECT_DIR)/*.o
	rm -rf $(BIN)/*