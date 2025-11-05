
## HomeSensorKernel – Linux Kernel Module `hcsr501`

This project contains a simple **Linux kernel module** (`hcsr501.ko`) designed to handle the **HC-SR501 PIR motion sensor**.
The provided Makefile supports building the module both **on the host machine (x86_64)** and **for Raspberry Pi 5 (aarch64, Yocto SDK)**.

---

## Requirements

### For host build

* A Linux system (e.g. Ubuntu, Debian)
* Installed kernel headers:

  ```bash
  sudo apt install linux-headers-$(uname -r)
  ```

### For Raspberry Pi 5 build (Yocto SDK)

* A working Yocto SDK including the kernel sources, for example `poky-rpi5-sdk`
* Before building, the SDK environment must be sourced:

  ```bash
  source /opt/poky-rpi5-sdk/environment-setup-cortexa76-poky-linux
  ```

---

## Building the module

### Host build (x86_64)

```bash
make host
```

* Builds the module against the currently running host kernel (`/lib/modules/$(uname -r)/build`).
* All build artifacts (`.ko`, `.o`, `.mod.c`, `.cmd`, `Module.symvers`, `modules.order`) are placed in:

  ```
  ./build/
  ```

### Raspberry Pi 5 build (Yocto SDK, aarch64)

```bash
make rpi5
```

* Builds the module using the Yocto SDK cross-compiler for ARM64.
* Before running this command, execute:

  ```bash
  source /opt/poky-rpi5-sdk/environment-setup-cortexa76-poky-linux
  ```
* The final kernel module will be located at:

  ```
  ./build/hcsr501.ko
  ```

---

## Cleaning

### Clean host build

```bash
make clean-host
```

### Clean Raspberry Pi 5 build

```bash
make clean-rpi5
```

### Clean all

```bash
make clean
```

Removes the `build/` directory and all intermediate files (`.o`, `.cmd`, `.mod.c`, `.symvers`, etc.).

---

## Project structure

```
HomeSensorKernel/
├── hcsr501.c         # kernel module source file
├── Makefile          # build script
└── build/            # (auto-created) build output directory
    ├── hcsr501.ko
    ├── hcsr501.o
    ├── hcsr501.mod.c
    ├── .hcsr501.o.cmd
    ├── modules.order
    └── Module.symvers
```

---

## Deploying to Raspberry Pi 5

After building, you can copy and load the module on the target device:

```bash
scp build/hcsr501.ko root@192.168.100.50:/home/root/
ssh root@192.168.100.50 "insmod /home/root/hcsr501.ko gpio=586"
```

To unload the module:

```bash
ssh root@192.168.100.50 "rmmod hcsr501"
```

---

## Additional information

* The file `hcsr501.c` implements GPIO handling and interrupt-based event detection for the HC-SR501 sensor.
* When loading the module, you must specify the GPIO number:

  ```bash
  insmod hcsr501.ko gpio=586
  ```
* The module creates a character device `/dev/hcsr501`, which provides access to the sensor state and motion detection events via standard read or poll operations.

---

