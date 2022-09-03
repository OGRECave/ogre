FROM quay.io/pypa/manylinux2014_x86_64:latest
RUN yum install -y zlib-devel libXrandr-devel mesa-libEGL-devel vulkan-devel
WORKDIR /
RUN curl -LO https://sdk.lunarg.com/sdk/download/1.3.224.1/linux/vulkansdk-linux-x86_64-1.3.224.1.tar.gz && tar xvf vulkansdk-linux-x86_64-1.3.224.1.tar.gz
ENV VULKAN_SDK=/1.3.224.1/x86_64/
WORKDIR /workspace
CMD PATH="/opt/python/$PYVER/bin/:$PATH" pip wheel . --verbose