CXX = g++
CXXFLAGS = -std=c++23 -Wall -Iexternal/spdlog/include -pthread 
LDFLAGS = -lsndfile

BUILD_DIR = build
TARGET = $(BUILD_DIR)/app

SRCS = main.cpp \
		server/http_request.cpp \
		server/tcp_connection.cpp \
		server/tcp_server.cpp \
		transform/pipeline.cpp \
		transform/effects/effect.cpp \
		external/stb/stb.cpp \
		transform/effects/grayscale.cpp \
		transform/effects/sepia.cpp \
		transform/effects/pixelate.cpp \
		transform/effects/invert.cpp \
		transform/audio_io.cpp \
		transform/effects/audio_effect.cpp \
		transform/effects/reverse.cpp \
		transform/effects/normalize.cpp \
		transform/effects/gaussian_blur.cpp \
		concurrency/thread_pool.cpp \
		server/http_response.cpp \
		transform/effects/ordered_dithering.cpp \
		transform/effects/floyd_steinberg.cpp \
		transform/effects/copy_image.cpp

OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)