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
RUN make clean && \
    make && \
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
    python3 \
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
# Start background watcher for restart trigger file (Python - no fork/exec per poll)\n\
# When trigger file is detected, kill PID 1 (radiod) to trigger container restart\n\
python3 -c "\n\
import os, signal, time\n\
TRIGGER = '"'"'/var/run/restart-trigger/restart'"'"'\n\
while True:\n\
    if os.path.exists(TRIGGER):\n\
        print('"'"'Restart trigger detected from ubersdr, restarting...'"'"', flush=True)\n\
        os.remove(TRIGGER)\n\
        os.kill(1, signal.SIGTERM)\n\
        break\n\
    time.sleep(2)\n\
" &\n\
\n\
# Start background thread-stats writer (Python - zero forks per cycle)\n\
# Samples /proc/<pid>/task/<tid>/stat twice over 2 seconds, writes CSV atomically.\n\
# Uses /dev/shm for the temp file (RAM, no disk writes) then os.replace to shared volume.\n\
# Output: /var/run/restart-trigger/radiod-thread-stats.csv\n\
# Format: name,cpu_pct,cpu_num  (one row per thread, header on every write)\n\
python3 -c "\n\
import os, re, time\n\
\n\
STATS_FILE = '"'"'/var/run/restart-trigger/radiod-thread-stats.csv'"'"'\n\
TMP_FILE   = '"'"'/var/run/restart-trigger/radiod-thread-stats.tmp'"'"'\n\
try:\n\
    HZ = os.sysconf(os.sysconf_names['"'"'SC_CLK_TCK'"'"'])\n\
except Exception:\n\
    HZ = 100\n\
\n\
# Greedy match strips "PID (name) " prefix correctly even if name contains ")"\n\
STAT_RE = re.compile(r'"'"'^\\d+ \\(.*\\) '"'"')\n\
\n\
def find_radiod_pid():\n\
    for entry in os.scandir('"'"'/proc'"'"'):\n\
        if not entry.name.isdigit():\n\
            continue\n\
        try:\n\
            with open(f'"'"'/proc/{entry.name}/comm'"'"') as f:\n\
                if f.read().strip() == '"'"'radiod'"'"':\n\
                    return entry.name\n\
        except OSError:\n\
            pass\n\
    return None\n\
\n\
def read_thread_stats(pid):\n\
    # Single pass: read both stat and comm for each thread in one loop.\n\
    # Returns dict of tid -> (utime, stime, cpu_num, name)\n\
    stats = {}\n\
    task_dir = f'"'"'/proc/{pid}/task'"'"'\n\
    try:\n\
        tids = os.listdir(task_dir)\n\
    except OSError:\n\
        return stats\n\
    for tid in tids:\n\
        try:\n\
            with open(f'"'"'{task_dir}/{tid}/stat'"'"') as f:\n\
                raw = f.read()\n\
            fields = STAT_RE.sub('"'"''"'"', raw).split()\n\
            try:\n\
                with open(f'"'"'{task_dir}/{tid}/comm'"'"') as f:\n\
                    name = f.read().strip()\n\
            except OSError:\n\
                name = '"'"'?'"'"'\n\
            stats[tid] = (int(fields[11]), int(fields[12]), fields[36], name)\n\
        except (OSError, IndexError, ValueError):\n\
            pass\n\
    return stats\n\
\n\
while True:\n\
    pid = find_radiod_pid()\n\
    if pid is None or not os.path.isdir(f'"'"'/proc/{pid}/task'"'"'):\n\
        time.sleep(1)\n\
        continue\n\
    snap1 = read_thread_stats(pid)\n\
    t1 = time.monotonic()\n\
    time.sleep(2)\n\
    snap2 = read_thread_stats(pid)\n\
    elapsed_ticks = max(1, (time.monotonic() - t1) * HZ)\n\
    lines = ['"'"'name,cpu_pct,cpu_num'"'"']\n\
    for tid, (u2, s2, cpu_num, name) in snap2.items():\n\
        if tid not in snap1:\n\
            continue\n\
        u1, s1, _, _ = snap1[tid]\n\
        pct = (u2 - u1 + s2 - s1) * 100.0 / elapsed_ticks\n\
        lines.append(f'"'"'{name},{pct:.1f},{cpu_num}'"'"')\n\
    with open(TMP_FILE, '"'"'w'"'"') as f:\n\
        f.write('"'"'\\n'"'"'.join(lines) + '"'"'\\n'"'"')\n\
    os.replace(TMP_FILE, STATS_FILE)\n\
" &\n\
\n\
# Start radiod as PID 1\n\
exec /usr/local/sbin/radiod "$@"\n\
' > /usr/local/bin/start-radiod.sh && \
    chmod +x /usr/local/bin/start-radiod.sh

# Default command - run startup script with config file
ENTRYPOINT ["/usr/local/bin/start-radiod.sh"]
CMD ["/etc/ka9q-radio/radiod@ubersdr.conf"]