FROM quay.io/pypa/manylinux_2_28_x86_64:latest
RUN yum install -y libXrandr-devel mesa-libEGL-devel vulkan-devel
WORKDIR /
RUN curl -LO https://sdk.lunarg.com/sdk/download/1.3.275.0/linux/vulkansdk-linux-x86_64-1.3.275.0.tar.xz && tar xvf vulkansdk-linux-x86_64-1.3.275.0.tar.xz
ENV VULKAN_SDK=/1.3.275.0/x86_64/
WORKDIR /workspace
CMD PATH="/opt/python/$PYVER/bin/:$PATH" pip wheel . --verbose