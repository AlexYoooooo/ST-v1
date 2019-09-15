# Phasor

The phasor is a program that used to decode the query.

The PC will send a query (instruction file) to the hardware that contains information about the speed, direction and distance of the motor. The instruction can be nested and looped.

The phasor program in the hardware (the MCU) will decode the query, then send command to motor (control the direction) and timer (control the speed).

------

phasor-orginal.c shows how the phasor program work in PC environment. Please compile this c program using gcc compiler. To discover this program, read the command and try modify the step[] array.

phasor-eventdriver.c is a modified version of the phasor. In practical, the MCU is event driven (the MCU drives the motor at t1, sets the timer and waits for t, then drives the motor again). This code will be embedded in the MCU's program.

test.c is a testbench used to verify the phasor-eventdriver.c. Compile and run this code using gcc, it sould give you the similar output like phasor-orginal.c.

sim.c is a program used to simulate the query output on PC. Please compile this program, used this program to verify your query before upload your query to the MCU. For instruction of this program, refer the comment in this c file.