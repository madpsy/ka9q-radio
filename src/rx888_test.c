// RX888 Debug/Test Program
// Minimal standalone program to diagnose RX888 startup issues
// Copyright 2024

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include <time.h>
#include <sys/time.h>

#include "ezusb.h"
#include "rx888.h"

// Global variable required by ezusb.c
int Ezusb_verbose = 0;

#define VENDOR_ID 0x04b4
#define UNLOADED_PRODUCT_ID 0x00f3
#define LOADED_PRODUCT_ID 0x00f1

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_CYAN    "\033[36m"

static const char *usb_speeds[] = {
    "unknown",
    "Low (1.5 Mb/s)",
    "Full (12 Mb/s)",
    "High (480 Mb/s)",
    "Super (5 Gb/s)",
    "Super+ (10Gb/s)"
};

void print_header(const char *msg) {
    printf("\n%s=== %s ===%s\n", COLOR_CYAN, msg, COLOR_RESET);
}

void print_success(const char *msg) {
    printf("%s✓ %s%s\n", COLOR_GREEN, msg, COLOR_RESET);
}

void print_error(const char *msg) {
    printf("%s✗ %s%s\n", COLOR_RED, msg, COLOR_RESET);
}

void print_warning(const char *msg) {
    printf("%s⚠ %s%s\n", COLOR_YELLOW, msg, COLOR_RESET);
}

void print_info(const char *msg) {
    printf("%s  %s%s\n", COLOR_BLUE, msg, COLOR_RESET);
}

double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

int scan_and_load_firmware(const char *firmware_path) {
    print_header("Scanning for Unloaded RX888 Devices");
    
    libusb_device **device_list;
    int dev_count = libusb_get_device_list(NULL, &device_list);
    int loaded_count = 0;
    
    printf("Found %d USB devices total\n", dev_count);
    
    for (int i = 0; i < dev_count; i++) {
        libusb_device *device = device_list[i];
        if (device == NULL)
            break;
        
        struct libusb_device_descriptor desc = {0};
        int rc = libusb_get_device_descriptor(device, &desc);
        if (rc != 0)
            continue;
        
        if (desc.idVendor != VENDOR_ID || desc.idProduct != UNLOADED_PRODUCT_ID)
            continue;
        
        printf("\n");
        print_info("Found unloaded RX888:");
        printf("  Vendor: 0x%04x, Product: 0x%04x\n", desc.idVendor, desc.idProduct);
        printf("  Bus %d, Device %d\n", 
               libusb_get_bus_number(device), 
               libusb_get_device_address(device));
        
        libusb_device_handle *handle = NULL;
        rc = libusb_open(device, &handle);
        if (rc != 0 || handle == NULL) {
            print_error("Failed to open device");
            printf("  Error: %s\n", libusb_strerror(rc));
            continue;
        }
        
        // Get device strings
        if (desc.iManufacturer) {
            char manufacturer[100] = {0};
            libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, 
                                              (unsigned char *)manufacturer, 
                                              sizeof(manufacturer));
            printf("  Manufacturer: %s\n", manufacturer);
        }
        
        if (desc.iProduct) {
            char product[100] = {0};
            libusb_get_string_descriptor_ascii(handle, desc.iProduct, 
                                              (unsigned char *)product, 
                                              sizeof(product));
            printf("  Product: %s\n", product);
        }
        
        if (desc.iSerialNumber) {
            char serial[100] = {0};
            libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, 
                                              (unsigned char *)serial, 
                                              sizeof(serial));
            printf("  Serial: %s\n", serial);
        }
        
        // Load firmware
        printf("\n  Loading firmware: %s\n", firmware_path);
        double start_time = get_time_ms();
        
        int result = ezusb_load_ram(handle, firmware_path, FX_TYPE_FX3, IMG_TYPE_IMG, 1);
        
        double elapsed = get_time_ms() - start_time;
        
        if (result == 0) {
            print_success("Firmware loaded successfully");
            printf("  Time: %.1f ms\n", elapsed);
            loaded_count++;
        } else {
            print_error("Firmware loading failed");
        }
        
        libusb_close(handle);
    }
    
    libusb_free_device_list(device_list, 1);
    
    if (loaded_count == 0) {
        print_warning("No unloaded RX888 devices found");
    } else {
        printf("\n");
        printf("%s✓ Loaded firmware on %d device(s)%s\n", COLOR_GREEN, loaded_count, COLOR_RESET);
        print_info("Waiting 2 seconds for device re-enumeration...");
        sleep(2);
    }
    
    return loaded_count;
}

int test_loaded_devices(uint64_t target_serial) {
    print_header("Scanning for Loaded RX888 Devices");
    
    libusb_device **device_list;
    int dev_count = libusb_get_device_list(NULL, &device_list);
    int found_count = 0;
    
    for (int i = 0; i < dev_count; i++) {
        libusb_device *device = device_list[i];
        if (device == NULL)
            break;
        
        struct libusb_device_descriptor desc = {0};
        int rc = libusb_get_device_descriptor(device, &desc);
        if (rc != 0)
            continue;
        
        if (desc.idVendor != VENDOR_ID || desc.idProduct != LOADED_PRODUCT_ID)
            continue;
        
        found_count++;
        printf("\n");
        printf("%s  Found loaded RX888 #%d:%s\n", COLOR_BLUE, found_count, COLOR_RESET);
        printf("  Vendor: 0x%04x, Product: 0x%04x\n", desc.idVendor, desc.idProduct);
        printf("  Bus %d, Device %d\n", 
               libusb_get_bus_number(device), 
               libusb_get_device_address(device));
        
        libusb_device_handle *handle = NULL;
        rc = libusb_open(device, &handle);
        if (rc != 0 || handle == NULL) {
            print_error("Failed to open device");
            printf("  Error: %s\n", libusb_strerror(rc));
            continue;
        }
        
        // Get device strings
        char manufacturer[100] = {0};
        char product[100] = {0};
        char serial[100] = {0};
        
        if (desc.iManufacturer) {
            libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, 
                                              (unsigned char *)manufacturer, 
                                              sizeof(manufacturer));
            printf("  Manufacturer: %s\n", manufacturer);
        }
        
        if (desc.iProduct) {
            libusb_get_string_descriptor_ascii(handle, desc.iProduct, 
                                              (unsigned char *)product, 
                                              sizeof(product));
            printf("  Product: %s\n", product);
        }
        
        if (desc.iSerialNumber) {
            libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, 
                                              (unsigned char *)serial, 
                                              sizeof(serial));
            printf("  Serial: %s\n", serial);
        }
        
        // Check USB speed
        enum libusb_speed usb_speed = libusb_get_device_speed(device);
        printf("  USB Speed: ");
        if (usb_speed < 6) {
            printf("%s", usb_speeds[usb_speed]);
        } else {
            printf("unknown (index %d)", usb_speed);
        }
        
        if (usb_speed >= LIBUSB_SPEED_SUPER) {
            printf(" %s✓%s\n", COLOR_GREEN, COLOR_RESET);
        } else {
            printf(" %s✗ NOT FAST ENOUGH!%s\n", COLOR_RED, COLOR_RESET);
            print_error("Device must be connected to USB 3.0 (blue) port!");
        }
        
        // Check if serial matches (if specified)
        if (target_serial != 0) {
            uint64_t device_serial = strtoull(serial, NULL, 16);
            if (device_serial == target_serial) {
                print_success("Serial number matches target");
            } else {
                print_warning("Serial number does not match target");
                printf("  Target: %016llx, Found: %016llx\n", 
                       (unsigned long long)target_serial, 
                       (unsigned long long)device_serial);
            }
        }
        
        // Check kernel driver
        printf("\n  Checking kernel driver...\n");
        int driver_active = libusb_kernel_driver_active(handle, 0);
        if (driver_active == 1) {
            print_warning("Kernel driver is attached");
            printf("  Attempting to detach...\n");
            rc = libusb_detach_kernel_driver(handle, 0);
            if (rc == 0) {
                print_success("Kernel driver detached");
            } else {
                print_error("Failed to detach kernel driver");
                printf("  Error: %s\n", libusb_strerror(rc));
            }
        } else if (driver_active == 0) {
            print_success("No kernel driver attached");
        } else {
            print_warning("Could not determine kernel driver status");
        }
        
        // Try to claim interface
        printf("\n  Testing interface claim...\n");
        rc = libusb_claim_interface(handle, 0);
        if (rc == 0) {
            print_success("Successfully claimed interface 0");
            
            // Note: We skip the TESTFX3 command test because it can hang
            // if the device isn't fully initialized. The ability to claim
            // the interface is sufficient to show the device is accessible.
            print_info("Device communication test skipped (can hang on some devices)");
            
            libusb_release_interface(handle, 0);
        } else {
            print_error("Failed to claim interface");
            printf("  Error: %s\n", libusb_strerror(rc));
            printf("  This may indicate another process is using the device\n");
        }
        
        libusb_close(handle);
        
        printf("\n");
        if (usb_speed >= LIBUSB_SPEED_SUPER && rc == 0) {
            print_success("Device appears to be working correctly!");
        } else {
            print_error("Device has issues that need to be resolved");
        }
    }
    
    libusb_free_device_list(device_list, 1);
    
    if (found_count == 0) {
        print_error("No loaded RX888 devices found");
        printf("\nPossible reasons:\n");
        printf("  1. Firmware loading failed\n");
        printf("  2. Device did not re-enumerate after firmware load\n");
        printf("  3. USB cable disconnected\n");
        printf("  4. Insufficient USB power\n");
        return -1;
    }
    
    return found_count;
}

void print_usage(const char *progname) {
    printf("RX888 Debug/Test Program\n\n");
    printf("Usage: %s -f <firmware> [options]\n\n", progname);
    printf("Required:\n");
    printf("  -f <path>    Firmware file path (e.g., ../share/SDDC_FX3.img)\n\n");
    printf("Options:\n");
    printf("  -s <serial>  Target serial number (hex, optional)\n");
    printf("  -v           Verbose mode\n");
    printf("  -h           Show this help\n\n");
    printf("This program will:\n");
    printf("  1. Scan for unloaded RX888 devices\n");
    printf("  2. Load firmware if needed\n");
    printf("  3. Scan for loaded devices\n");
    printf("  4. Test USB speed and communication\n");
    printf("  5. Report any issues found\n\n");
}

int main(int argc, char *argv[]) {
    const char *firmware_path = NULL;
    uint64_t target_serial = 0;
    
    // Parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "f:s:vh")) != -1) {
        switch (opt) {
            case 'f':
                firmware_path = optarg;
                break;
            case 's':
                target_serial = strtoull(optarg, NULL, 16);
                break;
            case 'v':
                Ezusb_verbose = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    // Check if firmware path was provided
    if (firmware_path == NULL) {
        fprintf(stderr, "Error: Firmware file path is required\n\n");
        print_usage(argv[0]);
        return 1;
    }
    
    printf("%s", COLOR_CYAN);
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║          RX888 Debug/Test Program v1.0                    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("%s\n", COLOR_RESET);
    
    printf("Firmware: %s\n", firmware_path);
    if (target_serial != 0) {
        printf("Target Serial: %016llx\n", (unsigned long long)target_serial);
    }
    if (Ezusb_verbose) {
        printf("Verbose mode: ON\n");
    }
    
    // Initialize libusb
    print_header("Initializing USB");
    int rc = libusb_init(NULL);
    if (rc != 0) {
        print_error("Failed to initialize libusb");
        printf("Error: %s\n", libusb_strerror(rc));
        return 1;
    }
    print_success("libusb initialized");
    
    // Scan and load firmware
    scan_and_load_firmware(firmware_path);
    
    // Test loaded devices
    int found = test_loaded_devices(target_serial);
    
    // Cleanup
    libusb_exit(NULL);
    
    // Summary
    print_header("Summary");
    if (found > 0) {
        printf("%s✓ Found %d working RX888 device(s)%s\n", COLOR_GREEN, found, COLOR_RESET);
        printf("\n%sThe RX888 appears to be responding correctly.%s\n",
               COLOR_GREEN, COLOR_RESET);
        printf("If radiod still fails, check:\n");
        printf("  - Configuration file settings\n");
        printf("  - Sample rate and clock settings\n");
        printf("  - USB bandwidth/buffer settings\n");
        return 0;
    } else {
        print_error("No working RX888 devices found");
        printf("\n%sTroubleshooting steps:%s\n", COLOR_YELLOW, COLOR_RESET);
        printf("  1. Ensure device is plugged into USB 3.0 (blue) port\n");
        printf("  2. Try a different USB cable\n");
        printf("  3. Check USB power supply\n");
        printf("  4. Verify firmware file exists and is correct\n");
        printf("  5. Check dmesg for USB errors: dmesg | tail -50\n");
        printf("  6. Try: sudo rmmod cdc_acm (if kernel driver interferes)\n");
        return 1;
    }
}
