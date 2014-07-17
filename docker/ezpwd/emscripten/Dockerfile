FROM		ubuntu:14.04
MAINTAINER	Perry Kundert "perry@hardconsulting.com"

RUN apt-get -y update					\
    && apt-get -y upgrade				\
    && apt-get install -y				\
	git build-essential cmake python nodejs		\
	openjdk-7-jdk clang-3.4 llvm-3.4		\
    && apt-get clean

RUN git clone --depth=1 --branch 1.20.0 https://github.com/kripken/emscripten /opt/emscripten

RUN for prog in em++ em-config emar emcc emconfigure emmake emranlib emrun emscons; do \
  ln -sf /opt/emscripten/$prog /usr/local/bin; done

ENV LLVM /usr/lib/llvm-3.4/bin

# partially forked from https://github.com/hanazuki/docker-ubuntu-emscripten/blob/master/Dockerfile
# build an example code in order to cache the standard libraries and relooper
RUN emcc --version && \
  mkdir -p /tmp/emscripten_temp && cd /tmp/emscripten_temp && \
  EMCC_FORCE_STDLIBS=1 em++ -O2 /opt/emscripten/tests/hello_world_sdl.cpp -o hello.js && \
  (nodejs hello.js 2> /dev/null | grep hello) && \
  rm -rf /tmp/emscripten_temp

