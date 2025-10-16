# KA9Q-Radio Docker Container

This Docker container provides a complete ka9q-radio installation configured to run with SDR hardware and communicate with ka9q_ubersdr via a shared bridge network.

## Quick Start

### Using Docker Compose with ka9q_ubersdr (Recommended)

This setup uses a shared external Docker network for multicast communication between ka9q-radio and ka9q_ubersdr.

1. **Create the shared network:**
```bash
docker network create sdr-network --subnet 172.20.0.0/16
```

2. **Start ka9q-radio:**
```bash
cd ~/repos/ka9q-radio/docker
docker-compose up -d
```

3. **Start ka9q_ubersdr (in another terminal):**
```bash
cd ~/repos/ka9q_ubersdr/docker
docker-compose up -d
```

4. **Access the web interface at `http://localhost:8080`**

5. **View logs:**
```bash
# ka9q-radio logs
docker-compose logs -f

# Or view both
docker logs -f ka9q-radio
docker logs -f ka9q_ubersdr
```

6. **Stop the containers:**
```bash
# Stop ka9q-radio
docker-compose down

# Stop ka9q_ubersdr (from its directory)
cd ~/repos/ka9q_ubersdr/docker
docker-compose down
```

### Standalone Usage (Without ka9q_ubersdr)

If you want to run ka9q-radio standalone without the web interface, you can still use the docker-compose setup, but you'll need to modify the network configuration to use host networking instead.

### Using Docker CLI

#### Build the Image

```bash
docker build -t ka9q-radio -f docker/Dockerfile .
```

### Run with UberSDR Configuration

The container is pre-configured to use `radiod@ubersdr.conf` which is set up for RX888 hardware.

**Basic run:**
```bash
docker run -d \
  --name ka9q-radio \
  --privileged \
  --network host \
  -v /dev/bus/usb:/dev/bus/usb \
  ka9q-radio
```

**Run with custom configuration:**
```bash
docker run -d \
  --name ka9q-radio \
  --privileged \
  --network host \
  -v /dev/bus/usb:/dev/bus/usb \
  -v /path/to/your/radiod@ubersdr.conf:/etc/ka9q-radio/radiod@ubersdr.conf \
  ka9q-radio
```

**Run with different config file:**
```bash
docker run -d \
  --name ka9q-radio \
  --privileged \
  --network host \
  -v /dev/bus/usb:/dev/bus/usb \
  -v /path/to/your/config.conf:/etc/ka9q-radio/radiod.conf \
  ka9q-radio /usr/local/sbin/radiod /etc/ka9q-radio/radiod.conf
```

## Network Configuration

### Bridge Network (Default - for use with ka9q_ubersdr)

The default docker-compose.yml uses a **shared external bridge network** (`sdr-network`):
- Allows multicast communication between ka9q-radio and ka9q_ubersdr containers
- Isolates SDR traffic from host network
- Requires creating the network first: `docker network create sdr-network --subnet 172.20.0.0/16`

### Host Network (Alternative - for standalone use)

If you want to use host networking instead (for standalone operation or integration with host-based applications):

1. Edit `docker-compose.yml` and change:
   ```yaml
   networks:
     - sdr-network
   ```
   to:
   ```yaml
   network_mode: host
   ```

2. Remove the `networks:` section at the bottom of the file

## Docker Run Options Explained

- `--privileged`: Required for USB device access and network capabilities
- `--network sdr-network`: Uses shared bridge network for multicast with ka9q_ubersdr
- `-v /dev/bus/usb:/dev/bus/usb`: Mounts USB devices for SDR hardware access
- `-v /path/to/config:/etc/ka9q-radio/radiod@ubersdr.conf`: Mounts your custom configuration

## Available Configuration Examples

The container includes example configurations in `/etc/ka9q-radio/examples/`:

- `radiod@ubersdr.conf` - RX888 with dynamic channel support (default)
- `radiod@airspy-generic.conf` - Airspy SDR
- `radiod@funcube-generic.conf` - FUNcube Dongle
- `radiod@sdrplay-generic.conf` - SDRplay devices
- And many more...

## Viewing Logs

```bash
docker logs ka9q-radio
```

## Stopping the Container

```bash
docker stop ka9q-radio
docker rm ka9q-radio
```

## Interactive Shell

To get a shell inside the running container:

```bash
docker exec -it ka9q-radio /bin/bash
```

## Notes

- The container requires `--privileged` mode for USB device access
- Bridge networking (`sdr-network`) is used for multicast communication with ka9q_ubersdr
- The shared network must be created before starting containers
- Make sure your SDR hardware is connected before starting the container
- The default configuration expects an RX888 device
- Configuration and data are persisted in Docker volumes (`radiod-config` and `radiod-data`)

### Configuration Persistence

The container uses a **persistent volume** for configuration files:
- On first run, the default `radiod@ubersdr.conf` and example configs are copied to the `radiod-config` volume
- Configuration changes made inside the container persist across restarts
- To edit configs: `docker exec -it ka9q-radio vi /etc/ka9q-radio/radiod@ubersdr.conf`
- To reset to defaults: `docker volume rm docker_radiod-config` (after stopping the container)
- To update configs after rebuilding the image: delete the volume and restart the container

This ensures that:
1. Configuration changes persist across container restarts
2. You can edit configs without rebuilding the image
3. New image builds with updated default configs will be used on fresh deployments

## Troubleshooting

**No SDR device found:**
- Ensure the device is connected: `lsusb`
- Check USB permissions on the host
- Verify the device is not in use by another process

**Multicast issues:**
- Ensure the `sdr-network` bridge network exists and both containers are connected
- Check that both containers can resolve each other via DNS (e.g., `docker exec ka9q_ubersdr ping ka9q-radio`)
- Verify multicast is working on the bridge network
- Check container logs for multicast address generation messages

## Hardware Support

The container includes drivers for:
- RX888
- Airspy/Airspy HF+
- RTL-SDR
- FUNcube Dongle
- SDRplay (if API is available)
- And other supported SDR hardware

## Configuration

The default configuration file is located at:
```
/etc/ka9q-radio/radiod@ubersdr.conf
```

You can override this by mounting your own configuration file or by passing a different config path as a command argument.