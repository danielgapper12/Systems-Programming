# CPTS 360: Programming Assignment 3 – Linux Kernel Module

This repository contains my solution for Programming Assignment 3 from CPTS 360 (Systems Programming). The goal of this project was to gain hands-on experience with Linux kernel development by building a custom kernel module that communicates with userspace via the `/proc` filesystem and tracks userspace CPU time for registered processes.

## Project Overview

The kernel module allows userspace applications to register their PID by writing to `/proc/kmlab/status`. The module tracks the CPU time (user time) of each registered process and updates this data every 5 seconds using a timer and workqueue. The current list of registered processes and their CPU times can be read from the same `/proc` entry.

This system uses:

- A custom `/proc` interface
- A kernel timer and workqueue
- Kernel linked lists to store process data
- Locking to handle concurrent access
- Communication between kernelspace and userspace via `copy_to_user()` and `copy_from_user()`

## Key Features

- Kernel module accepts PIDs from userspace via `/proc/kmlab/status`
- Periodically updates CPU usage of each registered process every 5 seconds
- Supports multiple registered processes simultaneously
- Safely handles process removal when a process terminates
- Protects critical regions using spinlocks
- Fully functional test application (`userapp.c`) that registers itself, runs a computation, and prints CPU time

## Skills Demonstrated

- Linux kernel module development and debugging
- Kernel-to-userspace communication using the proc filesystem
- Use of kernel timers, workqueues, and linked lists
- Synchronization via kernel locking primitives
- C systems programming in a constrained kernel environment

## Files

- `kmlab.c`: Kernel module implementation
- `userapp.c`: Userspace application for registration and testing
- `kmlab_test.sh`: Script to run and test module with multiple userspace processes
- `random-numbers`: Used for input consistency if required
- `pa3_gapper.pdf`: Project report outlining design choices and implementation strategy

## How to Build and Run

Compile the kernel module using the provided `Makefile`:

```bash
make
sudo insmod kmlab.ko        # Insert the module
echo $$ > /proc/kmlab/status  # Register the current shell's PID
cat /proc/kmlab/status     # View registered processes and their CPU usage
sudo rmmod kmlab           # Remove the module when done
