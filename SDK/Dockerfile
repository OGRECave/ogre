FROM quay.io/pypa/manylinux2014_x86_64:latest
RUN yum install -y zlib-devel libXrandr-devel mesa-libEGL-devel
ENV PATH "/opt/python/cp38-cp38/bin/:$PATH"
CMD pip wheel . --verbose