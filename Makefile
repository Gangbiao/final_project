all: main getAttribute getSpace
main: main.cpp
	g++ -o $@ $^ -std=c++11 -ldl -L/home/hadoop/workshop/RAMCloud/obj.master -lramcloud
getAttribute: getAttribute.cpp
	g++ -o $@ $^
getSpace: getSpace.cpp
	g++ -o $@ $^ -std=c++11 -ldl -L/home/hadoop/workshop/RAMCloud/obj.master -lramcloud
	#g++ -o test test.cpp -std=c++11 -ldl -lramcloud
clean:
	rm -rf main getAttribute getSpace
