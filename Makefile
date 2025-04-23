# 编译器和标志
CC = gcc
CFLAGS = `pkg-config --cflags gio-2.0 gio-unix-2.0` -I. -Wall
LDFLAGS = `pkg-config --libs gio-2.0 gio-unix-2.0` -lcjson

# 生成的GDBus代码
GDBUS_XML = com.example.gdbusdemo.xml
GDBUS_GENERATED = gdbus-generated.h gdbus-generated.c

# 目标文件
SERVER = server
CLIENT = client

all: $(GDBUS_GENERATED) $(SERVER) $(CLIENT)

# 生成GDBus代码
$(GDBUS_GENERATED): $(GDBUS_XML)
	gdbus-codegen --generate-c-code=gdbus-generated --c-namespace=ComExample --interface-prefix=com.example. $(GDBUS_XML)

# 编译服务器
$(SERVER): server.c $(GDBUS_GENERATED)
	$(CC) server.c gdbus-generated.c $(CFLAGS) $(LDFLAGS) -o $(SERVER)

# 编译客户端
$(CLIENT): client.c $(GDBUS_GENERATED)
	$(CC) client.c gdbus-generated.c $(CFLAGS) $(LDFLAGS) -o $(CLIENT)

# 清理
clean:
	rm -f $(SERVER) $(CLIENT) $(GDBUS_GENERATED) *.o

.PHONY: all clean