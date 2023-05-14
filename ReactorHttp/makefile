src=$(wildcard *.c)
obj=$(patsubst %.c, %.o, $(src))
target=server
$(target):$(obj)
	gcc $(obj) -o $(target) -lpthread

%.o:%.c
	gcc $< -c -std=c99

.PHONY:clean
clean:
	-rm $(obj) $(target) -f

