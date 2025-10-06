/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 * @author Jonathan Gitej
 * @author Atik Mahmud
 *
 */

#include "interrupts.hpp"

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;      //!< string to store single line of trace file
    std::string execution;  //!< string to accumulate the execution output

    /******************ADD YOUR VARIABLES HERE*************************/

    int cpu_time = 0;                       // To keep track of the current CPU time
    const int context_save_time = 10;       // Provided in the assignment description
    const int ISR_ACTIVITY_TIME = 40;       // Provided in the assignment description

    //Steps done in int_boilerplate function:

    //1. Switch to/from kernel mode, time = 1
    //2. Save/restore context, time = 10
    //3. Calculate where in memory the ISR start address is, time = 1
    //4. Get ISR address from vector table, time = 1
    //5. Perform the activity of the ISR, time = 40 (for each activity)
    //6. Perform IRET, time = 1

    // ISR of END I/O = SYSCALL

    /******************************************************************/

    //parse each line of the input trace file
    while(std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/

        // Added to handle malformed lines
        if (activity == "null") {
            continue;
        }

        // This is the CPU Burst
        if (activity == "CPU") {
            execution += std::to_string(cpu_time) + ", " + std::to_string(duration_intr) + ", CPU burst\n";
            cpu_time += duration_intr;
        } 

        else if (activity == "SYSCALL" || activity == "END_IO") {
            auto [isr_string, new_time] = intr_boilerplate(cpu_time, duration_intr, context_save_time, vectors);
            execution += isr_string;
            cpu_time = new_time;

            int time_left = delays[duration_intr];

            //std::min ensures that if the time left is less than 40, we only use the time left and vice versa
            int isr_time = std::min(time_left, ISR_ACTIVITY_TIME);
            execution += std::to_string(cpu_time) + ", " + std::to_string(isr_time) + ", " + activity + ": run the ISR (device driver)\n";
            cpu_time += isr_time;
            time_left -= isr_time;

            if (activity == "SYSCALL") {
                int transfer_time = std::min(time_left, ISR_ACTIVITY_TIME);
                execution += std::to_string(cpu_time) + ", " + std::to_string(transfer_time) + ", transfer data from device to memory\n";
                cpu_time += transfer_time;
                time_left -= transfer_time;
            
                execution += std::to_string(cpu_time) + ", " + std::to_string(time_left) + ", check for errors\n";
                cpu_time += time_left;

            } 
            
            if (activity == "END_IO") {
                execution += std::to_string(cpu_time) + ", " + std::to_string(time_left) + ", check device status\n";
                cpu_time += time_left;
            }

            // The example.txt posted in brightspace doesn't show IRET for END_IO nor SYSCALL. We're assumming it's needed for both cases.
            // IRET needed for SYSCALL to return to user mode
            // IRET needed for END_IO to return from the interrupt handler
            execution += std::to_string(cpu_time) + ", 1, IRET\n";
            cpu_time += 1;
        }
    }

    input_file.close();

    write_output(execution);

    return 0;
}
