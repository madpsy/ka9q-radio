# Multi-stage build for ka9q-radio on Ubuntu 24.04
FROM ubuntu:24.04 AS builder

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Enable universe repository and install build dependencies
RUN apt-get update && \
    apt-get install -y software-properties-common && \
    add-apt-repository universe && \
    apt-get update && \
    apt-get install -y \
    build-essential \
    git \
    rsync \
    libbsd-dev \
    libiniparser-dev \
    libopus-dev \
    libavahi-client-dev \
    libavahi-common-dev \
    libncurses5-dev \
    libncursesw5-dev \
    libportaudio2 \
    portaudio19-dev \
    libsamplerate0 \
    libsamplerate0-dev \
    libogg-dev \
    libvorbis-dev \
    libfftw3-dev \
    uuid-dev \
    avahi-utils \
    && rm -rf /var/lib/apt/lists/*

# Install SDR hardware libraries if available (may not be in all repos)
RUN apt-get update && \
    (apt-get install -y libairspyhf-dev || true) && \
    (apt-get install -y libairspy-dev || true) && \
    (apt-get install -y librtlsdr-dev || true) && \
    rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /build

# Copy ka9q-radio source
COPY . /build/ka9q-radio

# Build and install ka9q-radio
WORKDIR /build/ka9q-radio
RUN make && \
    # Create radio user first (needed by other install steps)
    /usr/sbin/adduser --quiet --system --group radio && \
    # Install src binaries
    cd src && make install && cd .. && \
    # Install share files
    cd share && make install && cd .. && \
    # Install service files (skip systemctl daemon-reload)
    cd service && rsync * /etc/systemd/system && cd .. && \
    # Install rules (create udev directory first)
    mkdir -p /etc/udev/rules.d && \
    cd rules && rsync * /etc/udev/rules.d && cd .. && \
    # Install docs
    cd docs && make install && cd .. && \
    # Install aux files (skip sysctl - can't run in Docker build)
    cd aux && \
    mkdir -p /etc/fftw /etc/radio /etc/avahi/services /var/lib/ka9q-radio /var/lib/hfdl /var/lib/ka9q-radio/ft8 /var/lib/ka9q-radio/ft4 /var/lib/ka9q-radio/wspr /var/log/acars && \
    chown -R radio:radio /etc/fftw /etc/radio /etc/avahi/services /var/lib/ka9q-radio /var/lib/hfdl /var/lib/ka9q-radio/ft8 /var/lib/ka9q-radio/ft4 /var/lib/ka9q-radio/wspr /var/log/acars && \
    chmod g+ws /etc/fftw /etc/radio /etc/avahi/services /var/lib/ka9q-radio /var/lib/hfdl /var/lib/ka9q-radio/ft8 /var/lib/ka9q-radio/ft4 /var/lib/ka9q-radio/wspr /var/log/acars && \
    touch /etc/avahi/hosts /var/log/hfdl.log && \
    chgrp radio /etc/avahi/hosts /var/log/hfdl.log && \
    chmod g+w /etc/avahi/hosts /var/log/hfdl.log && \
    rsync 98-sockbuf.conf 50-multicast.conf /etc/sysctl.d && \
    rsync airspy-blacklist.conf /etc/modprobe.d && \
    rsync aprsfeed.rotate ft8.rotate ft4.rotate wspr.rotate hfdl.rotate /etc/logrotate.d && \
    chown root:root /etc/logrotate.d/* && \
    chmod go-w /etc/logrotate.d/* && \
    rsync --ignore-existing systable.conf /var/lib/hfdl && \
    rsync start-hfdl.sh /usr/local/sbin && \
    rsync ka9q-cleanups /etc/cron.d/ && \
    chown root:root /etc/cron.d/ka9q-cleanups && \
    chmod 0644 /etc/cron.d/ka9q-cleanups && \
    rsync set_lo_multicast.sh /usr/local/bin

# Runtime stage
FROM ubuntu:24.04

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Enable universe repository and install runtime dependencies
RUN apt-get update && \
    apt-get install -y software-properties-common && \
    add-apt-repository universe && \
    apt-get update && \
    apt-get install -y \
    libopus0 \
    libavahi-client3 \
    libavahi-common3 \
    libncurses6 \
    libncursesw6 \
    libportaudio2 \
    libsamplerate0 \
    libogg0 \
    libvorbis0a \
    libvorbisenc2 \
    libfftw3-single3 \
    libuuid1 \
    avahi-utils \
    libbsd0 \
    libiniparser1 \
    ca-certificates \
    iproute2 \
    && rm -rf /var/lib/apt/lists/*

# Install SDR hardware runtime libraries if available
RUN apt-get update && \
    (apt-get install -y libairspyhf1 || true) && \
    (apt-get install -y libairspy0 || true) && \
    (apt-get install -y librtlsdr0 || true) && \
    rm -rf /var/lib/apt/lists/*

# Create necessary directories
RUN mkdir -p /var/lib/ka9q-radio \
    && mkdir -p /etc/ka9q-radio \
    && mkdir -p /usr/local/share/ka9q-radio

# Copy installed binaries from builder (make install puts them in /usr/local/sbin and /usr/local/bin)
# Daemons (in /usr/local/sbin)
COPY --from=builder /usr/local/sbin/radiod /usr/local/sbin/
COPY --from=builder /usr/local/sbin/aprs /usr/local/sbin/
COPY --from=builder /usr/local/sbin/aprsfeed /usr/local/sbin/
COPY --from=builder /usr/local/sbin/cwd /usr/local/sbin/
COPY --from=builder /usr/local/sbin/packetd /usr/local/sbin/

# Executables (in /usr/local/bin)
COPY --from=builder /usr/local/bin/control /usr/local/bin/
COPY --from=builder /usr/local/bin/fft-gen /usr/local/bin/
COPY --from=builder /usr/local/bin/jt-decoded /usr/local/bin/
COPY --from=builder /usr/local/bin/metadump /usr/local/bin/
COPY --from=builder /usr/local/bin/monitor /usr/local/bin/
COPY --from=builder /usr/local/bin/pcmrecord /usr/local/bin/
COPY --from=builder /usr/local/bin/pl /usr/local/bin/
COPY --from=builder /usr/local/bin/powers /usr/local/bin/
COPY --from=builder /usr/local/bin/tune /usr/local/bin/
COPY --from=builder /usr/local/bin/wd-record /usr/local/bin/

# Optional executables (may not exist on all platforms - set_xcvr only on Raspberry Pi)
# Note: This will fail silently if the file doesn't exist in the builder stage

# Make binaries executable
RUN chmod +x /usr/local/sbin/* /usr/local/bin/*

# Copy dynamic driver libraries (SDR hardware drivers)
COPY --from=builder /usr/local/lib/ka9q-radio/ /usr/local/lib/ka9q-radio/
RUN ln -sf /usr/local/lib/ka9q-radio/*.so /usr/local/lib/ 2>/dev/null || true

# Copy share files if they exist
COPY --from=builder /usr/local/share/ka9q-radio/ /usr/local/share/ka9q-radio/

# Copy configuration examples to a temporary location for initialization
COPY --from=builder /build/ka9q-radio/config /usr/local/share/ka9q-radio/config-templates/

# Set working directory
WORKDIR /etc/ka9q-radio

# Expose common ports
# 5004 - RTP audio (UDP)
# 5006 - RTP control (UDP)
EXPOSE 5004/udp 5006/udp

# Environment variables
ENV TZ=UTC

# Health check - verify radiod process is running
HEALTHCHECK --interval=30s --timeout=10s --start-period=10s --retries=3 \
    CMD pgrep radiod || exit 1

# Create startup script to initialize config volume, enable multicast, and start radiod
RUN echo '#!/bin/bash\n\
set -e\n\
\n\
# Initialize config directory from templates if empty or missing default config\n\
if [ ! -f /etc/ka9q-radio/radiod@ubersdr.conf ]; then\n\
    echo "Initializing config directory from templates..."\n\
    mkdir -p /etc/ka9q-radio/examples\n\
    cp -r /usr/local/share/ka9q-radio/config-templates/* /etc/ka9q-radio/examples/\n\
    cp /etc/ka9q-radio/examples/radiod@ubersdr.conf /etc/ka9q-radio/radiod@ubersdr.conf\n\
    echo "Config directory initialized with default ubersdr configuration"\n\
else\n\
    echo "Using existing configuration from persistent volume"\n\
fi\n\
\n\
# Enable multicast on eth0 interface if it exists\n\
if ip link show eth0 >/dev/null 2>&1; then\n\
    echo "Enabling multicast on eth0..."\n\
    ip link set eth0 multicast on || true\n\
    ip link set eth0 allmulticast on || true\n\
fi\n\
\n\
# Also enable on loopback for local communication\n\
ip link set lo multicast on || true\n\
\n\
# Create restart trigger directory\n\
mkdir -p /var/run/restart-trigger\n\
\n\
# Record our startup time and signal ubersdr\n\
RADIOD_START_TIME=$(date +%s)\n\
echo "$RADIOD_START_TIME" > /var/run/restart-trigger/radiod-startup-time\n\
touch /var/run/restart-trigger/restart-ubersdr\n\
echo "Radiod started at $RADIOD_START_TIME, signaling ubersdr"\n\
\n\
# Start background watcher for restart trigger file\n\
# When trigger file is detected, kill PID 1 (radiod) to trigger container restart\n\
(\n\
    while true; do\n\
        if [ -f /var/run/restart-trigger/restart ]; then\n\
            echo "Restart trigger detected from ubersdr, restarting..."\n\
            rm -f /var/run/restart-trigger/restart\n\
            kill -TERM 1 || kill -KILL 1\n\
            exit 0\n\
        fi\n\
        sleep 0.5\n\
    done\n\
) &\n\
\n\
# Start radiod as PID 1\n\
exec /usr/local/sbin/radiod "$@"\n\
' > /usr/local/bin/start-radiod.sh && \
    chmod +x /usr/local/bin/start-radiod.sh

# Default command - run startup script with config file
ENTRYPOINT ["/usr/local/bin/start-radiod.sh"]
CMD ["/etc/ka9q-radio/radiod@ubersdr.conf"]