src=$(wildcard *.cpp)
obj=$(patsubst %.cpp, %.o, $(src))
target=server.app
$(target):$(obj)
	g++ $(obj) -o $(target) -lpthread -lmysqlclient  -L/usr/lib64/mysql -ljsoncpp

%.o:%.cpp
	g++ $< -c -std=c++11

.PHONY:clean
clean:
	-rm $(obj) $(target) -f
