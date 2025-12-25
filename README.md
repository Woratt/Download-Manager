#Download Manager (Qt/C++)

**Download Manager** is a professional desktop download manager built with **Qt 6**, designed to provide high performance, stability, and a modern user experience.  
The project demonstrates solid knowledge of **multithreading**, **asynchronous I/O**, and **clean software architecture**.

---
![Application Screenshot](image/image_git.png)

## Features

- **Parallel Downloads** — custom thread pool for handling multiple downloads simultaneously without blocking the UI  
- **Resume Support** — pause and resume downloads using the HTTP `Range` header  
- **Dynamic Optimization** — automatic adjustment of buffer size and timeouts based on network speed  
- **Smart Retries** — retry mechanism with exponential backoff on connection failures  
- **Persistent Storage** — full **SQLite** integration to restore download queue between application restarts  
- **Conflict Handling** — URL duplication checks and automatic file name conflict resolution  

---

## Project Architecture

The project follows **Separation of Concerns** and a modular architecture:

| Class | Responsibility |
| --- | --- |
| **`DownloadManager`** | Main controller: queue management, conflict handling, UI ↔ database communication |
| **`DownloadTask`** | Network operations, double buffering logic, disk writing |
| **`ThreadPool`** | Dynamic task distribution across `QThread` instances |
| **`DownloadDatabase`** | SQLite data access layer using `DownloadRecord` objects |
| **`DownloadAdapter`** | Adapter pattern: synchronizes states between core logic, UI, and database |
| **`DownloadItem`** | UI component for displaying progress and controlling downloads |

---

## Technical Highlights

### Double Buffering

The downloader uses a **double buffering** mechanism (`swapBuffers`):  
while one buffer receives data from the network, the second buffer is asynchronously written to disk.  
This approach minimizes I/O latency and improves overall throughput.

### Thread Pool Management

Instead of creating a new thread for each download, the application uses a **limited thread pool** based on `QThread::idealThreadCount`, which:

- reduces memory fragmentation  
- prevents excessive OS-level context switching  
- improves scalability under load  

### Adaptive Buffer Algorithm

The `adjustBufferSize` method dynamically adapts the buffer size based on real-time download speed:

- **Low speed** → small buffer for stability  
- **High speed** → buffer up to **256 KB** to reduce system write calls  

---

## Technology Stack

- **Language:** C++17  
- **Framework:** Qt 6 (Core, Network, Widgets, SQL)  
- **Database:** SQLite 3  
- **Platforms:** Windows · Linux · macOS  

---

## Build & Run

1. Install **Qt 6.5+** and a compiler with **C++17** support  
2. Clone the repository:
   ```bash
   git clone git@github.com:Woratt/Download-Manager.git
   mkdir build
   cd build
   cmake ..
   cmake --build .

