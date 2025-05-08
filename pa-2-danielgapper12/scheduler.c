#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Headers as needed

typedef enum {false, true} bool;        // Allows boolean types in C

/* Defines a job struct */
typedef struct Process {
    uint32_t A;                         // A: Arrival time of the process
    uint32_t B;                         // B: Upper Bound of CPU burst times of the given random integer list
    uint32_t C;                         // C: Total CPU time required
    uint32_t M;                         // M: Multiplier of CPU burst time
    uint32_t processID;                 // The process ID given upon input read

    uint8_t status;                     // 0 is unstarted, 1 is ready, 2 is running, 3 is blocked, 4 is terminated

    int32_t finishingTime;              // The cycle when the the process finishes (initially -1)
    uint32_t currentCPUTimeRun;         // The amount of time the process has already run (time in running state)
    uint32_t currentIOBlockedTime;      // The amount of time the process has been IO blocked (time in blocked state)
    uint32_t currentWaitingTime;        // The amount of time spent waiting to be run (time in ready state)

    uint32_t IOBurst;                   // The amount of time until the process finishes being blocked
    uint32_t CPUBurst;                  // The CPU availability of the process (has to be > 1 to move to running)

    int32_t quantum;                    // Used for schedulers that utilise pre-emption

    bool isFirstTimeRunning;            // Used to check when to calculate the CPU burst when it hits running mode

    uint32_t originalC;                 // Original total CPU time required
    uint32_t originalA;                 // Original arrival time
    bool finished;                      // Flag indicating if the process has finished

    struct Process* nextInBlockedList;  // A pointer to the next process available in the blocked list
    struct Process* nextInReadyQueue;   // A pointer to the next process available in the ready queue
    struct Process* nextInReadySuspendedQueue; // A pointer to the next process available in the ready suspended queue
} _process;


uint32_t CURRENT_CYCLE = 0;             // The current cycle that each process is on
uint32_t TOTAL_CREATED_PROCESSES = 0;   // The total number of processes constructed
uint32_t TOTAL_STARTED_PROCESSES = 0;   // The total number of processes that have started being simulated
uint32_t TOTAL_FINISHED_PROCESSES = 0;  // The total number of processes that have finished running
uint32_t TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED = 0; // The total cycles in the blocked state

const char* RANDOM_NUMBER_FILE_NAME= "random-numbers";
const uint32_t SEED_VALUE = 200;  // Seed value for reading from file


/**
 * Reads a random non-negative integer X from a file with a given line named random-numbers (in the current directory)
 */
uint32_t getRandNumFromFile(uint32_t line, FILE* random_num_file_ptr){
    uint32_t end, loop;
    char str[512];

    rewind(random_num_file_ptr); // reset to be beginning
    for(end = loop = 0;loop<line;++loop){
        if(0==fgets(str, sizeof(str), random_num_file_ptr)){ //include '\n'
            end = 1;  //can't input (EOF)
            break;
        }
    }
    if(!end) {
        return (uint32_t) atoi(str);
    }

    // fail-safe return
    return (uint32_t) 1804289383;
}



/**
 * Reads a random non-negative integer X from a file named random-numbers.
 * Returns the CPU Burst: : 1 + (random-number-from-file % upper_bound)
 */
uint32_t randomOS(uint32_t upper_bound, uint32_t process_indx, FILE* random_num_file_ptr)
{
    char str[20];

    //uint32_t unsigned_rand_int = (uint32_t) getRandNumFromFile(process_indx, random_num_file_ptr); // I changed to

    uint32_t unsigned_rand_int = (uint32_t) getRandNumFromFile(SEED_VALUE+process_indx, random_num_file_ptr);
    uint32_t returnValue = 1 + (unsigned_rand_int % upper_bound);

    return returnValue;
} 


/********************* SOME PRINTING HELPERS *********************/


/**
 * Prints to standard output the original input
 * process_list is the original processes inputted (in array form)
 */
void printStart(_process process_list[])
{
    printf("The original input was: %i", TOTAL_CREATED_PROCESSES);

    uint32_t i = 0;
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        printf(" ( %i %i %i %i)", process_list[i].A, process_list[i].B,
               process_list[i].C, process_list[i].M);
    }
    printf("\n");
} 

/**
 * Prints to standard output the final output
 * finished_process_list is the terminated processes (in array form) in the order they each finished in.
 */
void printFinal(_process finished_process_list[])
{
    printf("The (sorted) input is: %i", TOTAL_CREATED_PROCESSES);

    uint32_t i = 0;
    for (; i < TOTAL_FINISHED_PROCESSES; ++i)
    {
        printf(" ( %i %i %i %i)", finished_process_list[i].A, finished_process_list[i].B,
               finished_process_list[i].C, finished_process_list[i].M);
    }
    printf("\n");
} // End of the print final function

/**
 * Prints out specifics for each process.
 * @param process_list The original processes inputted, in array form
 */
void printProcessSpecifics(_process process_list[])
{
    uint32_t i = 0;
    printf("\n");
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        uint32_t turnaround_time = process_list[i].finishingTime - process_list[i].originalA;
        uint32_t waiting_time = turnaround_time - process_list[i].currentCPUTimeRun - process_list[i].currentIOBlockedTime;

        printf("Process %i:\n", process_list[i].processID);
        printf("\t(A,B,C,M) = (%i,%i,%i,%i)\n", process_list[i].originalA, process_list[i].B,
               process_list[i].originalC, process_list[i].M);
        printf("\tFinishing time: %i\n", process_list[i].finishingTime);
        printf("\tTurnaround time: %i\n", turnaround_time);
        printf("\tI/O time: %i\n", process_list[i].currentIOBlockedTime);
        printf("\tWaiting time: %i\n", waiting_time);
        printf("\n");
    }
}


/**
 * Prints out the summary data
 * process_list The original processes inputted, in array form
 */
void printSummaryData(_process process_list[])
{
    uint32_t i = 0;
    double total_amount_of_time_utilizing_cpu = 0.0;
    double total_amount_of_time_io_blocked = 0.0;
    double total_amount_of_time_spent_waiting = 0.0;
    double total_turnaround_time = 0.0;
    uint32_t final_finishing_time = CURRENT_CYCLE; // - 1;
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        total_amount_of_time_utilizing_cpu += process_list[i].currentCPUTimeRun;
        total_amount_of_time_io_blocked += process_list[i].currentIOBlockedTime;
        total_amount_of_time_spent_waiting += process_list[i].currentWaitingTime;
        total_turnaround_time += (process_list[i].finishingTime - process_list[i].A);
    }

    // Calculates the CPU utilisation
    double cpu_util = total_amount_of_time_utilizing_cpu / final_finishing_time;

    // Calculates the IO utilisation
    double io_util = (double) TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED / final_finishing_time;

    // Calculates the throughput (Number of processes over the final finishing time times 100)
    double throughput =  100 * ((double) TOTAL_CREATED_PROCESSES/ final_finishing_time);

    // Calculates the average turnaround time
    double avg_turnaround_time = total_turnaround_time / TOTAL_CREATED_PROCESSES;

    // Calculates the average waiting time
    double avg_waiting_time = total_amount_of_time_spent_waiting / TOTAL_CREATED_PROCESSES;

    printf("Summary Data:\n");
    printf("\tFinishing time: %i\n", CURRENT_CYCLE); // was CURRENT_CYCLE - 1
    printf("\tCPU Utilisation: %6f\n", cpu_util);
    printf("\tI/O Utilisation: %6f\n", io_util);
    printf("\tThroughput: %6f processes per hundred cycles\n", throughput);
    printf("\tAverage turnaround time: %6f\n", avg_turnaround_time);
    printf("\tAverage waiting time: %6f\n", avg_waiting_time);
} // End of the print summary data function

/**
 * Helper function to sort processes by arrival time (A).
 */
void sortProcessesByArrival(_process process_list[], int num_processes) 
{
    for (int i = 0; i < num_processes - 1; i++) 
    {
        for (int j = 0; j < num_processes - i - 1; j++) 
        {
            if (process_list[j].A > process_list[j + 1].A) 
            {
                // Swap processes
                _process temp = process_list[j];
                process_list[j] = process_list[j + 1];
                process_list[j + 1] = temp;
            }
        }
    }
}


/**
 * Function to parse the input file and populate the process list.
 * @param filename The input filename
 * @param process_list The array of processes to be populated, or NULL if just counting processes.
 * @return Number of processes parsed, or -1 on error
 */
int parseInputFile(const char* filename, _process* process_list) 
{
    FILE* file = fopen(filename, "r");
    if (file == NULL) 
    {
        printf("Error: Could not open file %s\n", filename);
        return -1;
    }

    int num_processes;
    fscanf(file, "%d", &num_processes);  // Read number of processes
    TOTAL_CREATED_PROCESSES = num_processes;

    for (int i = 0; i < num_processes; i++) 
    {
        int A, B, C, M;
        fscanf(file, " (%d %d %d %d)", &A, &B, &C, &M);

        // Populate the process struct
        process_list[i].A = A;
        process_list[i].B = B;
        process_list[i].C = C;
        process_list[i].M = M;
        process_list[i].processID = i;
        process_list[i].status = 0;  // Unstarted
        process_list[i].finishingTime = -1;  // Not yet finished
        process_list[i].currentCPUTimeRun = 0;
        process_list[i].currentIOBlockedTime = 0;
        process_list[i].currentWaitingTime = 0;
        process_list[i].originalA = A;
        process_list[i].originalC = C;
        process_list[i].finished = false;

    }

    fclose(file);
    return num_processes;
}

void resetProcesses(_process* process_list, int num_processes) 
{
    for (int i = 0; i < num_processes; i++) 
    {
        process_list[i].A = process_list[i].originalA;
        process_list[i].C = process_list[i].originalC;
        process_list[i].status = 0;  // Unstarted
        process_list[i].finishingTime = -1;
        process_list[i].currentCPUTimeRun = 0;
        process_list[i].currentIOBlockedTime = 0;
        process_list[i].currentWaitingTime = 0;
        process_list[i].IOBurst = 0;
        process_list[i].CPUBurst = 0;
        process_list[i].finished = false;
    }
}



void scheduleFCFS(_process* process_list, int num_processes) 
{
    printf("######################### START OF FIRST COME FIRST SERVE #########################\n");

    uint32_t current_time = 0;  
    TOTAL_FINISHED_PROCESSES = 0;  
    TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED = 0; 

    printStart(process_list);   // Print the original input

    // Sort the processes by arrival time
    sortProcessesByArrival(process_list, num_processes);

    // Open the random numbers file
    FILE* random_num_file_ptr = fopen(RANDOM_NUMBER_FILE_NAME, "r");
    if (random_num_file_ptr == NULL) 
    {
        printf("Error: Could not open random numbers file.\n");
        exit(1);
    }

    uint32_t rand_num_line = 1;  // Initialize random number line tracker

    // Initialize variables for process execution
    int running_process = -1;  // Index of currently running process
    bool processes_left = true;

    while (processes_left) 
    {
        for (int i = 0; i < num_processes; i++) 
        {
            if (process_list[i].status == 0 && process_list[i].A <= current_time) 
            {
                process_list[i].status = 1;  // Ready
            }
        }

        // If no process is running, pick the next one
        if (running_process == -1) {
            for (int i = 0; i < num_processes; i++) 
            {
                if (process_list[i].status == 1) {
                    running_process = i;
                    process_list[i].status = 2;  // Running

                    // Generate CPU burst
                    uint32_t cpu_burst = randomOS(process_list[i].B, rand_num_line++, random_num_file_ptr);

                    if (cpu_burst > (process_list[i].C - process_list[i].currentCPUTimeRun)) 
                    {
                        cpu_burst = process_list[i].C - process_list[i].currentCPUTimeRun;
                    }
                    process_list[i].CPUBurst = cpu_burst;
                    break;
                }
            }
        }

        // Check for process arrivals
        for (int i = 0; i < num_processes; i++) 
        {
            if (process_list[i].status == 0 && process_list[i].A <= current_time) 
            {
                process_list[i].status = 1;  // Ready
            }
            else if (process_list[i].status == 1) 
            {
                process_list[i].currentWaitingTime++;
            }
            else if (process_list[i].status == 2) 
            {
                process_list[i].CPUBurst--;
                process_list[i].currentCPUTimeRun++;
            }
            else if (process_list[i].status == 3) 
            {  
                // Blocked
                process_list[i].IOBurst--;
                process_list[i].currentIOBlockedTime++;
                TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED++;
                if (process_list[i].IOBurst == 0) 
                {
                    process_list[i].status = 1;  // Ready
                }
            }
        }

        

        // Execute the running process
        if (running_process != -1) {
            _process* p = &process_list[running_process];

            if (p->CPUBurst == 0) 
            {
                if (p->currentCPUTimeRun == p->C) 
                {
                    p->status = 4;  // Terminated
                    p->finishingTime = current_time + 1;
                    TOTAL_FINISHED_PROCESSES++;
                    running_process = -1;
                } else {
                    // Generate I/O burst
                    uint32_t io_burst = randomOS(p->B, rand_num_line++, random_num_file_ptr) * p->M;

                    p->IOBurst = io_burst;
                    p->status = 3;  // Blocked
                    running_process = -1;
                }
            }
        }

        // Check if all processes are finished
        processes_left = false;
        for (int i = 0; i < num_processes; i++) 
        {
            if (process_list[i].status != 4) 
            {
                processes_left = true;
                break;
            }
        }
        current_time++;
    }

    // Close the random numbers file
    fclose(random_num_file_ptr);

    // Update CURRENT_CYCLE with the final time
    CURRENT_CYCLE = current_time;  

    // Print sorted input
    printFinal(process_list);

    // Summary data
    printf("\nThe scheduling algorithm used was First Come First Serve\n");

    // Print detailed information for each process
    printProcessSpecifics(process_list);

    // Print summary data for all processes
    printSummaryData(process_list);

    printf("######################### END OF FIRST COME FIRST SERVE #########################\n");
}



// Round Robin scheduling algorithm, converted from FCFS
void scheduleRR(_process* process_list, int num_processes) 
{
    printf("######################### START OF ROUND ROBIN #########################\n");

    uint32_t current_time = 0;  
    TOTAL_FINISHED_PROCESSES = 0;  
    TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED = 0; 

    printStart(process_list);   // Print the original input

    // Sort the processes by arrival time
    sortProcessesByArrival(process_list, num_processes);

    // Open the random numbers file
    FILE* random_num_file_ptr = fopen(RANDOM_NUMBER_FILE_NAME, "r");
    if (random_num_file_ptr == NULL) {
        printf("Error: Could not open random numbers file.\n");
        exit(1);
    }

    uint32_t rand_num_line = 1;  // Initialize random number line tracker

    // Initialize variables for process execution
    int running_process = -1;  // Index of currently running process
    bool processes_left = true;


    while (processes_left) 
    {
        for (int i = 0; i < num_processes; i++) 
        {
            if (process_list[i].status == 0 && process_list[i].A <= current_time) 
            {
                process_list[i].status = 1;  // Ready
            }
        }

        // If no process is running, pick the next one
        if (running_process == -1) 
        {
            for (int i = 0; i < num_processes; i++) 
            {
                if (process_list[i].status == 1) 
                {
                    running_process = i;
                    process_list[i].status = 2;  // Running

                    // Generate CPU burst
                    uint32_t cpu_burst = randomOS(process_list[i].B, rand_num_line++, random_num_file_ptr);

                    if (cpu_burst > (process_list[i].C - process_list[i].currentCPUTimeRun)) 
                    {
                        cpu_burst = process_list[i].C - process_list[i].currentCPUTimeRun;
                    }
                    process_list[i].CPUBurst = cpu_burst;
                    break;
                }
            }
        }

        // Check for process arrivals
        for (int i = 0; i < num_processes; i++) {
            if (process_list[i].status == 0 && process_list[i].A <= current_time) 
            {
                process_list[i].status = 1;  // Ready
            }
            else if (process_list[i].status == 1) 
            {
                process_list[i].currentWaitingTime++;
            }
            else if (process_list[i].status == 2) 
            {
                process_list[i].CPUBurst--;
                process_list[i].currentCPUTimeRun++;
            }
            else if (process_list[i].status == 3) 
            {  
                // Blocked
                process_list[i].IOBurst--;
                process_list[i].currentIOBlockedTime++;
                TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED++;
                if (process_list[i].IOBurst == 0) 
                {
                    process_list[i].status = 1;  // Ready
                }
            }
        }

        

        // Execute the running process
        if (running_process != -1) 
        {
            _process* p = &process_list[running_process];

            if (p->CPUBurst == 0) 
            {
                if (p->currentCPUTimeRun == p->C) 
                {
                    p->status = 4;  // Terminated
                    p->finishingTime = current_time + 1;
                    TOTAL_FINISHED_PROCESSES++;
                    running_process = -1;
                } else {
                    // Generate I/O burst
                    uint32_t io_burst = randomOS(p->B, rand_num_line++, random_num_file_ptr) * p->M;

                    p->IOBurst = io_burst;
                    p->status = 3;  // Blocked
                    running_process = -1;
                }
            }
        }

        // Check if all processes are finished
        processes_left = false;
        for (int i = 0; i < num_processes; i++) 
        {
            if (process_list[i].status != 4) 
            {
                processes_left = true;
                break;
            }
        }
        current_time++;
    }

    // Close the random numbers file
    fclose(random_num_file_ptr);

    // Update CURRENT_CYCLE with the final time
    CURRENT_CYCLE = current_time;  

    // Print sorted input
    printFinal(process_list);

    // Summary data
    printf("\nThe scheduling algorithm used was Round Robin\n");

    // Print detailed information for each process
    printProcessSpecifics(process_list);

    // Print summary data for all processes
    printSummaryData(process_list);

    printf("######################### END OF ROUND ROBIN #########################\n");
}


void scheduleSJF(_process* process_list, int num_processes) 
{
    printf("######################### START OF SHORTEST JOB FIRST #########################\n");

    uint32_t current_time = 0;  
    TOTAL_FINISHED_PROCESSES = 0;  
    TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED = 0; 

    printStart(process_list);   // Print the original input

    // Open the random numbers file
    FILE* random_num_file_ptr = fopen(RANDOM_NUMBER_FILE_NAME, "r");
    if (random_num_file_ptr == NULL) 
    {
        printf("Error: Could not open random numbers file.\n");
        exit(1);
    }

    uint32_t rand_num_line = 1;  // Initialize random number line tracker

    // Initialize variables for process execution
    int running_process = -1;  // Index of currently running process

    while (TOTAL_FINISHED_PROCESSES < num_processes) 
    {
        // Check for process arrivals
        for (int i = 0; i < num_processes; i++) {
            if (process_list[i].status == 0 && process_list[i].originalA <= current_time) 
            {
                process_list[i].status = 1;  // Ready
            }
        }

        // Handle I/O Bursts (Blocked Processes)
        bool io_happening = false;
        for (int i = 0; i < num_processes; i++) {
            if (process_list[i].status == 3) {  // Blocked
                io_happening = true;
                process_list[i].IOBurst--;
                process_list[i].currentIOBlockedTime++;
                if (process_list[i].IOBurst == 0) {
                    process_list[i].status = 1;  // Ready
                }
            }
        }
        if (io_happening) 
        {
            TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED++;
        }

        // Update Running Process
        if (running_process != -1) 
        {
            _process* p = &process_list[running_process];
            p->CPUBurst--;
            p->currentCPUTimeRun++;
            p->C--;  // Decrement remaining CPU time
            if (p->CPUBurst == 0) 
            {
                if (p->C == 0) {
                    p->status = 4;  // Terminated
                    p->finishingTime = current_time;
                    p->finished = true;
                    TOTAL_FINISHED_PROCESSES++;
                    running_process = -1;
                } else {
                    // Generate I/O burst
                    uint32_t io_burst = randomOS(p->B, rand_num_line++, random_num_file_ptr) * p->M;
                    p->IOBurst = io_burst;
                    p->status = 3;  // Blocked
                    running_process = -1;
                }
            }
        }

        // Select Next Process to Run
        if (running_process == -1) 
        {
            int next_process = -1;
            uint32_t shortest_total_cpu_time = UINT32_MAX;

            for (int i = 0; i < num_processes; i++) 
            {
                if (process_list[i].status == 1) 
                { 
                    // Ready
                    if (process_list[i].originalC < shortest_total_cpu_time) 
                    {
                        shortest_total_cpu_time = process_list[i].originalC;
                        next_process = i;
                    }
                }
            }

            if (next_process != -1) 
            {
                running_process = next_process;
                process_list[running_process].status = 2; // Running
                // Generate CPU burst
                uint32_t cpu_burst = randomOS(process_list[running_process].B, rand_num_line++, random_num_file_ptr);
                if (cpu_burst > process_list[running_process].C) 
                {
                    cpu_burst = process_list[running_process].C;
                }
                process_list[running_process].CPUBurst = cpu_burst;
            }
        }

        // Update Waiting Times
        for (int i = 0; i < num_processes; i++) 
        {
            if (process_list[i].status == 1) 
            {
                process_list[i].currentWaitingTime++;
            }
        }

        // Advance Time
        current_time++;
    }

    // Close the random numbers file
    fclose(random_num_file_ptr);

    // Update CURRENT_CYCLE with the final time
    CURRENT_CYCLE = current_time - 1;  // Adjusted to fix finishing time

    // Print sorted input
    printFinal(process_list);

    // Print detailed information for each process
    printProcessSpecifics(process_list);

    // Summary data
    printf("\nThe scheduling algorithm used was Shortest Job First\n");

    // Print summary data for all processes
    printSummaryData(process_list);

    printf("######################### END OF SHORTEST JOB FIRST #########################\n");
}


/**
 * The magic starts from here
 */
int main(int argc, char *argv[])
{
   if (argc < 2) 
   {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];

    // Create a process list
    _process process_list[10]; 
    int num_processes = parseInputFile(filename, process_list);

    if (num_processes <= 0) {
        printf("Error: No processes found in input file\n");
        return 1;
    }

    scheduleFCFS(process_list, num_processes);

    resetProcesses(process_list, num_processes);

    scheduleRR(process_list, num_processes);

    resetProcesses(process_list, num_processes);

    scheduleSJF(process_list, num_processes); 
    
    return 0;
}
