# Audio/Image processor

A C++ Media Processing server written from scratch (apart from decoding/encoding of audio and images and, logging)
After running the server, upload a file on the HTML webpage, select a pipeline of effects, press apply and voila!

### Why write from scratch?
Most of the problems that one might face writing something like this are solved problems but I chose to write everything from scratch mostly to just expose myself to domains like networking or concurrency.

### How to Build

**Requirements:**
- g++ with C++23 support
- make
- libsndfile

**Install dependencies (Debian/Ubuntu):**
```bash
sudo apt update
sudo apt install g++ make libsndfile1-dev
```

**Build:**
```bash
make
```

**Run:**
```bash
./app
```

### Image effects implemented
- Sepia
- Pixelation
- Invertion
- Grayscale
- Gaussian Blur

### Audio effects implemented
- Reverse
- Normalization

Hoping to add at-least 1 FFT based effect for both images and audios 