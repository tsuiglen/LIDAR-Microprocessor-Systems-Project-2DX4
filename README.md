# LIDAR-Microprocessor-Systems-Project-2DX4
This device is an embedded spatial measurement system that utilizes time-of-flight to measure distance 360 degrees along the Y-Z plane. The stored data can be used to construct a 3D model of the samples for presentation purposes using applications on a personal computer. This system is similar to Light Detection and Ranging (LIDAR) equipment...
# Features
This device is an embedded spatial measurement system that utilizes time-of-flight to measure distance 360 degrees along the Y-Z plane. The stored data can be used to construct a 3D model of the samples for presentation purposes using applications on a personal computer. This system is similar to Light Detection and Ranging (LIDAR) equipment but commercial LIDAR products are often large in size and expensive in price. This device is a smaller and less expensive alternative for indoor spatial measurements.
# Hardware/Software Specifications
* • Texas Instruments MSP-EXP432E401Y Microcontroller
  * o 120MHz Default Bus Speed with individualized Bus Speed of 30 MHz
  * o Input Power Supply 3.3 VDC
  * o 1024KB Flash Memory
  * o 256KB of SRAM with Single-Cycle Access
  * o 6KB EEPROM
  * o Two 12-Bit SAR-Based ADC Modules (2 Msps)
  * o Uses programming language C in Keil software for implementation
* • VL53L1X Time-Of-Flight laser-ranging sensor
  * o Utilizes I2C interface (up to 400 kHz)
  * o Accurate distance ranging up to 400cm
* • 28BYJ-48 Stepper Motor with Velleman VMA401 ULN2003 Motor Driver
* • 115200 baud rate for serial communication in RealTerm software
* • Anaconda software for Python Open3D point cloud visualization
* • Meshlab software for conversion to .stl file
* • Total Cost: $145.00

# General Description
This digital device undergoes 4 main stages: Data Collection, Data Handling, Data Writing, and Data Visualization. Data Collection begins by acquiring samples of a number of distance measurements by the Time-Of-Flight sensor per revolution of the stepper motor. The Time-Of-Flight sensor works by calculating the distance from the time it takes for a laser to hit a target and reflect back. The Time-Of-Flight sensor automatically performs transduction, signal conditioning, and analog/digital conversion automatically. Each measurement uses the I2C connection between the sensor and the microcontroller to serially transmit data under a baud rate of 115200. At the end of each revolution, the user can press the button again to start another series of measurements while incrementing the X-value. Before the data is displayed on the RealTerm software, the sample distance measurements are processed to calculate the corresponding Y and Z values within the Keil software. The phase of processing of data is called Data Handling. When the user has accumulated a sufficient number of measurements, the Data Writing phase begins. The X, Y, and Z coordinates can be retrieved from RealTerm and written into an .xyz file which will be used in the final phase which is Data Visualization. The .xyz file is used by the Python library Open3D to visualize a point cloud by connecting the points of each plane and connecting the planes together with lines. For a proper 3D visualization, the software MeshLab can be used to create a mesh which can be exported as an .stl file for presentation.

# Device Characteristics
First, the Time-Of-Flight sensor uses 4 pins for SCL, SDA, Vin, and GND; which is connected to PB2, PB3, 5V, and GND respectively on the microcontroller. For the Motor Driver, the pins IN1, IN2, IN3, IN4, V+, and V- are connected to PM2, PM3, PM4, PM5, 5V, and GND respectively on the microcontroller. The port used for the external LED for displacement status is PL4 and the on-board LED port used for distance status is PN0. The port used for the button is PM0 and the individualized bus speed used is 30MHz. Refer to Figure 11: Circuit Schematic for the full circuit schematic. PB2 and PB3 are two of the ten I2C-enabled pins for serial communication and allows for four different transmission speeds (100kbps, 400kbps, 1Mbps, and 3.33Mbps). The communication speed is standard 100kbps. The following programs were used with the Windows operating system. The availability of these software programs for other operating systems are unknown. The main software for configuring and programming the microcontroller is Keil (the correct debugger CMSIS-DAP Debugger and JTAG Adapter XDS110 with CMSIS-DAP must be selected). For serial communication and displaying of data, RealTerm must be installed. Ensure that the pySerial library is installed to locate the port used for communication. Python 3.6.0 must be installed with the Python libraries numpy and Open3D 0.9.0.0 for Point Cloud
