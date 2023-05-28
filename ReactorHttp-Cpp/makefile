src=$(wildcard *.cpp)
obj=$(patsubst %.cpp, %.o, $(src))
target=server.app
$(target):$(obj)
	g++ $(obj) -o $(target) -lpthread

%.o:%.cpp
	g++ $< -c -std=c++11

.PHONY:clean
clean:
	-rm $(obj) $(target) -f
