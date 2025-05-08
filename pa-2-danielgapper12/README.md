# CPTS 360: Programming Assignment 2 – CPU Scheduling Simulator

This repository contains my solution for **Programming Assignment 2** from CPTS 360 (Systems Programming). The goal of this project was to implement and compare multiple CPU scheduling algorithms in C, focusing on time simulation, process management, and algorithmic decision-making.

## Project Overview

The program simulates three CPU scheduling algorithms:

- **FCFS (First Come First Serve)**
- **RR (Round Robin)** with quantum = 2
- **SJF (Shortest Job First)** (non-preemptive)

Each process is defined by four values: arrival time, CPU burst range, total CPU time, and I/O multiplier. The simulation progresses in discrete time units and tracks state transitions (ready, running, blocked, terminated) for all processes.

## Key Features

- **Accurate Time Simulation**: Simulates scheduling in real time with correct process state transitions.
- **Three Algorithm Modes**: FCFS, Round Robin (q=2), and non-preemptive SJF.
- **Tie-Breaking Logic**: Implements deterministic tie-breaking rules to ensure consistent results.
- **Metrics Output**: Calculates and prints finishing time, turnaround time, I/O time, waiting time, CPU utilization, and throughput.
- **File-Based Input**: Reads process descriptions from a file and uses a standardized `randomOS()` function.

## Skills Demonstrated

- Proficiency in **C programming**, especially with process/state modeling
- Implementation of classic **CPU scheduling algorithms**
- **File parsing** and random input handling
- **Structured debugging and testing** using sample inputs
- Strong understanding of **operating system concepts** such as scheduling fairness, blocking, and turnaround analysis

## Files

- `scheduler.c`: Main implementation of the simulator
- `random-numbers`: Random input used by the `randomOS()` function
- `Makefile`: For compiling and testing against sample inputs
- `pa2_gapper.pdf`: Brief write-up describing implementation decisions and tie-breaking strategies

## How to Run

```bash
make        # Builds the program
make test04 # Runs with sample input 4
