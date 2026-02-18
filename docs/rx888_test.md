# RX888 Debug/Test Program

## Overview

`rx888_test` is a standalone diagnostic tool for troubleshooting RX888 startup issues. It performs the same USB initialization and firmware loading that `radiod` does, but with detailed diagnostic output at each step.

## Purpose

This tool helps diagnose why an RX888 device might not be responding at startup by:

1. Scanning for unloaded RX888 devices (before firmware is loaded)
2. Loading the firmware onto any found devices
3. Scanning for loaded RX888 devices (after firmware)
4. Testing USB speed (must be SuperSpeed/USB 3.0)
5. Testing basic device communication
6. Reporting any issues found

## Building

The program is compiled from source:

```bash
cd src
gcc -std=gnu11 -Wall -O2 -o rx888_test rx888_test.c ezusb.c -lusb-1.0 -lm
```

## Usage

### Basic Usage

Run without arguments to test all RX888 devices:

```bash
sudo ./rx888_test
```

**Note:** Root/sudo access is typically required for USB device access.

### Command Line Options

```
Usage: rx888_test [options]

Options:
  -f <path>    Firmware file path (default: ../share/SDDC_FX3.img)
  -s <serial>  Target serial number (hex, optional)
  -v           Verbose mode
  -h           Show help
```

### Examples

Test with default firmware:
```bash
sudo ./rx888_test
```

Specify firmware location:
```bash
sudo ./rx888_test -f /usr/local/share/ka9q-radio/SDDC_FX3.img
```

Test specific device by serial number:
```bash
sudo ./rx888_test -s 0123456789ABCDEF
```

Verbose mode (shows detailed USB operations):
```bash
sudo ./rx888_test -v
```

## What It Tests

### 1. USB Initialization
- Verifies libusb can initialize
- Reports any USB subsystem issues

### 2. Unloaded Device Detection
- Scans for devices with vendor ID 0x04b4, product ID 0x00f3
- Reports manufacturer, product, and serial number strings
- Shows USB bus and device numbers

### 3. Firmware Loading
- Loads `SDDC_FX3.img` firmware onto unloaded devices
- Reports loading time and success/failure
- Uses the same firmware file as `radiod`

### 4. Loaded Device Detection
- Waits for device re-enumeration after firmware load
- Scans for devices with vendor ID 0x04b4, product ID 0x00f1
- Reports device information

### 5. USB Speed Check
- **Critical Test**: Verifies device is running at SuperSpeed (USB 3.0) or faster
- RX888 requires USB 3.0 for proper operation
- Reports if device is on wrong USB port

### 6. Kernel Driver Check
- Checks if kernel driver is attached to device
- Attempts to detach if necessary
- Reports if another process might be using the device

### 7. Interface Claim Test
- Attempts to claim USB interface 0
- Reports if another process has the device open

### 8. Communication Test
- Sends TESTFX3 command to device
- Verifies basic USB communication works

## Interpreting Results

### Success Indicators (Green ✓)
- libusb initialized
- Firmware loaded successfully
- Device found at SuperSpeed
- Interface claimed successfully
- TESTFX3 command succeeded

### Error Indicators (Red ✗)
- Failed to open device
- Firmware loading failed
- Device not at SuperSpeed (wrong USB port!)
- Failed to claim interface
- Communication test failed

### Warning Indicators (Yellow ⚠)
- No unloaded devices found (may already be loaded)
- Kernel driver attached (usually handled automatically)
- Serial number mismatch (if -s specified)

## Common Issues and Solutions

### Issue: "No unloaded RX888 devices found"
**Possible causes:**
- Device already has firmware loaded
- Device not plugged in
- USB cable disconnected
- Insufficient USB power

**Solution:** Check USB connection, try unplugging and replugging

### Issue: "USB Speed: High (480 Mb/s) ✗ NOT FAST ENOUGH!"
**Cause:** Device plugged into USB 2.0 port instead of USB 3.0

**Solution:** 
- Plug device into a **blue** USB 3.0 port
- Check that USB cable supports USB 3.0 (should have blue connector)

### Issue: "No loaded RX888 devices found"
**Possible causes:**
- Firmware loading failed
- Device did not re-enumerate after firmware load
- USB cable disconnected during firmware load
- Insufficient USB power

**Solution:** 
- Check dmesg for USB errors: `dmesg | tail -50`
- Try different USB cable
- Try different USB port
- Check USB power supply

### Issue: "Failed to claim interface"
**Cause:** Another process (like radiod) is using the device

**Solution:**
- Stop radiod: `sudo systemctl stop radiod@<instance>`
- Check for other processes: `lsusb -v | grep -A 5 "04b4:00f1"`

### Issue: "Kernel driver is attached"
**Cause:** Linux kernel has loaded a driver for the device

**Solution:** Usually handled automatically, but if not:
```bash
sudo rmmod cdc_acm
```

## Comparison with radiod

This test program performs the **same** initialization steps as `radiod`:

1. **Same firmware file**: Uses `SDDC_FX3.img` (default from line 213 of rx888.c)
2. **Same USB vendor/product IDs**: 0x04b4:0x00f3 (unloaded), 0x04b4:0x00f1 (loaded)
3. **Same firmware loading function**: Uses `ezusb_load_ram()` from ezusb.c
4. **Same USB requirements**: Requires SuperSpeed (USB 3.0) or faster

The key difference is that `rx888_test` provides **detailed diagnostic output** at each step, making it easier to identify exactly where the initialization fails.

## When to Use This Tool

Use `rx888_test` when:

- `radiod` fails to start with RX888
- RX888 device not responding
- Debugging USB connection issues
- Verifying firmware loading works
- Checking if device is on correct USB port
- Testing after hardware changes

## Exit Codes

- **0**: Success - device(s) found and working
- **1**: Failure - no working devices found or errors occurred

## Technical Details

- **Language**: C (C11 standard)
- **Dependencies**: libusb-1.0
- **Size**: ~39KB compiled
- **Lines of code**: ~420 lines
- **Execution time**: 2-5 seconds typical

## See Also

- [`rx888.c`](../src/rx888.c) - Main RX888 driver for radiod
- [`ezusb.c`](../src/ezusb.c) - Cypress EZ-USB firmware loader
- [`rx888.md`](SDR/rx888.md) - RX888 hardware documentation
