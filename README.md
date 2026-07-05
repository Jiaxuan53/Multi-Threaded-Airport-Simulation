# ✈️ Multi-Threaded Airport Simulation (C & pthreads)

![C](https://img.shields.io/badge/Language-C-blue.svg)
![pthreads](https://img.shields.io/badge/Concurrency-pthreads-orange.svg)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20WSL-lightgrey.svg)
![Build](https://img.shields.io/badge/Build-GCC-success.svg)
![Status](https://img.shields.io/badge/Status-Completed-brightgreen.svg)

---

## 🌍 Overview
This project is a **multi-threaded airport simulation written in C**, designed to demonstrate **concurrent programming**, **thread synchronization**, and **resource management** using **POSIX threads (pthreads)**, **mutexes**, and **semaphores**.

It simulates an airport environment where multiple aircraft coordinate with a **Control Tower** and a **Ground Operator** to safely manage:

✈️ Landing → 🚕 Taxiing → 🅿️ Gate docking → 🛫 Takeoff

The simulation ensures safe operations without **race conditions, deadlocks, or resource conflicts**.

---

## 🚀 Features

### 🧵 Multi-Threaded System
- Up to **10 aircraft threads**
- Dedicated threads for:
  - 🧭 Control Tower
  - 🧑‍✈️ Ground Operator
  - ✈️ Aircraft (Planes)

### 🔒 Synchronization & Safety
- Binary & counting semaphores
- Mutex-protected shared resources
- Safe concurrent execution

### 🛑 Resource Constraints
- 🛬 **Runway:** Only 1 plane at a time
- 🚕 **Taxiway:** Maximum of 3 planes
- 🅿️ **Gates:** 3 available gates (A, B, C)

### 🎨 Thread-Safe Logging
- Prevents mixed console output
- Color-coded terminal logs for clarity:
  - ✈️ Plane actions
  - 🧭 Tower decisions
  - 🧑‍✈️ Operator assignments

---

## 📋 Requirements

- 🐧 Linux / macOS / WSL (POSIX-compliant OS)
- 🧰 GCC Compiler
- 🧵 pthread library

---

## 🛠️ Build & Run

### 1️⃣ Compile the Program
```bash
gcc airport_solution.c -o airport_solution -pthread
