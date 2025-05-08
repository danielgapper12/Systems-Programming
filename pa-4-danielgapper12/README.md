# CPTS 360: Programming Assignment 4 – Web Proxy

This repository contains my solution for Programming Assignment 4 from CPTS 360 (Systems Programming). The objective of this project was to build a basic web proxy server in C capable of handling HTTP/1.0 GET requests. This assignment introduced core networking concepts such as socket programming, HTTP protocol parsing, and request forwarding.

## Project Overview

A web proxy acts as an intermediary between a web browser and a web server. When a browser sends a request, the proxy intercepts it, processes the request, and forwards it to the appropriate server. Upon receiving the server's response, the proxy relays it back to the browser. 

This proxy implementation:
- Accepts incoming HTTP/1.1 GET requests from clients
- Parses the request and extracts the target host, port, and path
- Sends a modified HTTP/1.0 GET request to the end server
- Forwards the response back to the client
- Adds essential headers: `Host`, `User-Agent`, `Connection`, and `Proxy-Connection`
- Handles malformed input and closes connections gracefully

## Key Features

- Parses full URLs and correctly extracts host, port (default 80 or specified), and path
- Translates HTTP/1.1 requests into HTTP/1.0 format
- Maintains robustness against malformed or invalid requests
- Manages binary and text-based content from end servers
- Modular design for parsing, request forwarding, and response handling

## Skills Demonstrated

- Socket programming using `accept()`, `connect()`, `send()`, and `recv()` in C
- Understanding of HTTP request/response structure
- Use of helper tools like `telnet`, `curl`, and `netcat` for testing
- Parsing and rewriting network protocol headers
- Managing client-server interactions through custom-built server software

## Files

- `proxy.c`: Main proxy implementation
- `csapp.c` and `csapp.h`: Utility functions for robust I/O (from CSAPP textbook)
- `Makefile`: Build instructions
- `pa4_gapper.pdf`: Implementation write-up and demonstration summary

## How to Build and Run

### Build the proxy:
```bash
make
