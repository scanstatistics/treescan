FROM rockylinux:8

# Necessary libraries
RUN    dnf -y install dnf-plugins-core \
    && dnf config-manager --set-enabled powertools \
    && dnf -y install \
         gcc-toolset-11 \
         make \
         libstdc++ \
         tar \
         which \
         curl \
         tzdata \
    && dnf clean all \
    && rm -rf /var/cache/dnf

# Set timezone
RUN    ln -sf /usr/share/zoneinfo/America/New_York /etc/localtime \
    && echo "America/New_York" > /etc/timezone 

# Install boost from source to get 1.90.0
RUN    mkdir -p /opt/boost \
    && curl -L https://archives.boost.io/release/1.90.0/source/boost_1_90_0.tar.gz | tar -xz -C /opt/boost --strip-components=1

# Install Java from source to get 25 & add symlinks
RUN    curl -L https://api.adoptium.net/v3/binary/latest/25/ga/linux/x64/jdk/hotspot/normal/eclipse -o /tmp/jdk.tar.gz \
    && mkdir -p /opt/java \
    && tar -xzf /tmp/jdk.tar.gz -C /opt/java --strip-components=1 \
    && rm /tmp/jdk.tar.gz 

# Create symlinks & env vars for overridden gcc and java where the build script expects them
RUN    mkdir -p /etc/alternatives/java_sdk \
    && ln -s /opt/java/include /etc/alternatives/java_sdk/include \
    && ln -s /opt/java/include/linux /etc/alternatives/java_sdk/include/linux \
    && ln -s /opt/rh/gcc-toolset-11/root/usr/bin/g++ /usr/bin/g++

ENV JAVA_HOME=/opt/java
ENV PATH="$JAVA_HOME/bin:$PATH"

WORKDIR /workspace/scripts/linux

ENTRYPOINT ["/bin/bash"]
CMD ["docker_build.sh"]
