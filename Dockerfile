FROM ubuntu:25.04
WORKDIR /app

RUN apt-get update

RUN apt-get install -y \
    software-properties-common wget gpg \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update 
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    ninja-build \
    pkg-config \
    curl \
    libtool \
    wget \
    gpg \
    zip \
    python3-pip \
    libltdl-dev \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update 

RUN apt-get install -y libx11-dev libxft-dev libxext-dev libwayland-dev libxkbcommon-dev libegl1-mesa-dev libibus-1.0-dev

RUN apt-get install -y \
    libltdl-dev python3-pip

RUN pip3 install Jinja2 --break-system-packages

COPY . .

RUN cmake -S . -B build -G Ninja \
    -DCMAKE_C_COMPILER=/usr/bin/gcc \
    -DCMAKE_CXX_COMPILER=/usr/bin/g++ \
    -DCMAKE_TOOLCHAIN_FILE=/app/vcpkg/scripts/buildsystems/vcpkg.cmake \ 
    -DVCPKG_TARGET_TRIPLET=x64-linux \
    -DVCPKG_MANIFEST_INSTALL=ON

RUN cmake --build build --config Release