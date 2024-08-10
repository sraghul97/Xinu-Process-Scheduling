# Xinu-Process-Scheduling

This repository is dedicated to implementing and testing various process management and scheduling algorithms in the Xinu operating system, as part of an Operating Systems Design course. The project is structured into three main parts:

Understanding Xinu’s Process Management: This phase involves studying and modifying Xinu’s process management code to understand process creation, scheduling, context switching, and process termination. Key tasks include implementing functions to track process runtime, turnaround time, and context switches, as well as enhancing debugging output for context switches.

Lottery Scheduling Implementation: This phase focuses on implementing the Lottery Scheduling algorithm in Xinu. It includes functions to manage process tickets, integrate the random number generator for ticket drawing, and modify the scheduler to support lottery-based process selection while maintaining system process priorities. The implementation also requires an analysis of fairness through runtime comparisons and graphical plotting.

Multi-Level Feedback Queue (MLFQ) Scheduling (ECE565 only): For graduate students, this part extends the scheduler to support the MLFQ policy. This involves implementing priority levels with time allotments, handling priority upgrades and downgrades, and configuring system parameters. The MLFQ scheduler adjusts priorities based on process behavior, providing a more nuanced approach to scheduling compared to Lottery Scheduling.

The repository includes all necessary code modifications, detailed reports on implementation approaches, and fairness analyses. It follows strict academic integrity guidelines, emphasizing original code and thorough testing on the provided VCL image.
