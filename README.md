Multi-Threaded Airport Simulation (C & pthreads)
✈️ Overview
This project is a multi-threaded airport simulation written in C. It serves as a practical demonstration of concurrent programming and process synchronization using POSIX threads (pthreads), semaphores, and mutexes.

The simulation models an airport environment where multiple planes (threads) must coordinate with a control tower and a ground operator to safely land, taxi, dock at a gate, and eventually take off without causing collisions or deadlocks.

🚀 Features
Thread Synchronization: Utilizes both binary and counting semaphores alongside mutex locks to manage shared resources (runways, taxiways, and gates).

Multi-Actor System: Features distinct worker threads for the Tower, the Operator, and up to 10 Planes.

Resource Constraints:

Runway: Only 1 plane can land or take off at a time.

Taxiway: Maximum capacity of 3 planes.

Gates: 3 available gates (A, B, and C).

Safe Console Output: Uses a dedicated mutex to prevent garbled standard output, complete with color-coded terminal messages to easily distinguish between actors.

📋 System Requirements
A POSIX-compliant operating system (Linux, macOS, WSL on Windows).

GCC (GNU Compiler Collection) or any standard C compiler.

The pthread library.

🛠️ How to Build and Run
1. Compile the Code
To compile the source code, open your terminal and use gcc. You must link the pthread library:

Bash
gcc airport_solution.c -o airport_solution -pthread
2. Execute the Simulation
Run the compiled executable, passing the number of planes (between 4 and 10) as a command-line argument:

Bash
./airport_solution <number_of_planes>
Example:

Bash
./airport_solution 5
🏗️ How It Works (System Architecture)
The program orchestrates three main types of threads:

1. The Planes (4 to 10 Threads)
Approaching: A plane requests landing clearance from the Tower and waits.

Landing: Once cleared, the plane occupies the runway, lands, and then releases the runway.

Taxiing: The plane enters the taxiway queue and signals the Operator.

Docking: Waits for the Operator to assign an available gate (A, B, or C). Once docked, it releases its taxiway slot.

Departure: After resting at the gate, the plane requests the runway, takes off, releases both the gate and the runway, and terminates.

2. The Control Tower (1 Thread)
Constantly listens for landing requests from approaching planes.

Ensures there is available space in the taxiway before granting landing permission.

Ensures the runway is clear, then signals the specific plane that it is safe to land.

3. The Ground Operator (1 Thread)
Monitors the taxiway queue.

When a plane enters the queue, the operator assigns it to the next available gate (using a round-robin style progression across gates A, B, and C).

Signals the specific plane with its gate assignment so it can dock.

🔒 Synchronization Primitives Used
Semaphores (sem_t):

runway: Binary semaphore ensuring mutual exclusion on the runway.

taxiway_slots: Counting semaphore limiting the taxiway to 3 planes.

request_landing / permit_landing: Synchronization between Planes and the Tower.

plane_in_queue / gate_assigned: Synchronization between Planes and the Operator.

gates[3]: Represents the availability of the 3 terminal gates.

Mutexes (pthread_mutex_t):

queue_mutex: Protects read/write access to the shared taxiway array.

request_mutex: Protects read/write access to the shared landing queue array.

print_mutex: Ensures that terminal printf statements from different threads do not interleave.
