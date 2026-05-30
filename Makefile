CXX = g++
CXXFLAGS = -std=c++17 -Wall

TARGET = app

SRCS = main.cpp server/http_request.cpp server/tcp_connection.cpp server/tcp_server.cpp
OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAG) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)