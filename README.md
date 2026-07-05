# ENES100 Water Mission Robot

Autonomous robot built for the ENES100 Water Mission using an Arduino Mega 2560. The system integrates motor control, environmental sensing, and autonomous navigation to complete a multi-stage mission involving water classification, depth measurement, and sample collection.

# Project Overview
The robot autonomously navigates a structured environment to complete three mission objectives:

1. Water Type Detection using a color sensor (TCS3200)
2. Water Depth Measurement using ultrasonic sensing
3. Sample Collection using servo-actuated mechanism and vacuum pump

All mission logic is coordinated through a state-based navigation system integrated with real-time position tracking via the ENES100 vision system.

# Hardware Platform
- Arduino Mega 2560
- TCS3200 Color Sensor
- HC-SR04 Ultrasonic Sensors (Front + Bottom)
- 4-Motor Drive System (Mecanum-style control)
- Servo Motor (Sample Collection Gate)
- Vacuum Pump
- ENES100 Vision System Module

# Software Architecture

The code is structured into modular subsystems:

1. Navigation System
  - Vision-based position tracking (Enes100.getX/Y/theta)
  - Stage-based autonomous state machine
  - Corridor alignment and goal-directed movement
  - Angle correction using proportional steering logic
2. Motor Control
  - Independent control of 4-wheel drivetrain
  - Forward, backward, and differential turning logic
  - Speed-controlled via PWM signals
3. Water Type Detection
  - RGB frequency measurement via TCS3200
  - Polluted vs clean classification based on calibrated thresholds
4. Depth Measurement
  - Ultrasonic distance sensing
  - Converts time-of-flight into millimeter depth estimation
  - Handles sensor noise and missing echo cases
5. Sample Collection System
  - Servo-controlled intake gate
  - Vacuum pump activation sequence
  - Timed collection cycle automation

# Mission Workflow
The robot executes missions in the following order:

1. Navigation Phase
  - Align to corridor
  - Traverse map using vision feedback
  - Reach mission zone
2. Water Classification
  - Read RGB values
  - Determine pollution status
  - Submit result via ENES100 API
3. Depth Measurement
  - Measure bottom distance
  - Compute water depth in mm
  - Report to system
4. Sample Collection
  - Open collection gate
  - Activate pump for timed cycle
  - Close mechanism and finalize mission

# Key Features
- Fully autonomous state-machine navigation
- Real-time vision-based localization
- Multi-sensor integration (color + ultrasonic)
- Modular mission execution system
- Fail-safe motor stopping and sensor validation

# Challenges Addressed
- Sensor noise filtering (color + ultrasonic)
- Position drift correction using vision feedback
- Coordinated multi-actuator timing
- Reliable state transitions in dynamic environment

# Author
Tidal Terps – ENES100 Team Project
University of Maryland, College Park
