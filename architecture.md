## Ideas For System Architecture

### Whiteboard
The whiteboard is what we use for global telemetry tracking in the system. This is a popular dichotomy used in industry. Basically how this works is we create a global repository for all telemetry across the system. These variables can be read, write, or both. Variables can be used as both commanding variables (turn a system on and off, set rates, send resets, etc) or for reading sensor data (getting IMU roll, pitch, yaw,  gps ECEF, barometer readings, etc). At each control cycle we will read and write these values, do some work upon these readings, and then telemeter them through downlink to the ground.


### Main Control Loop
- This main control loop can run at a set refresh rate (needs to be determined by how often we want telemetry and by the publish rate of the radio, we can only telemeter as fast as the radio can send data)

- Inside a main control loop we have three main steps
    - Each hardware driver calls its dispatch() method
    - We package and telemeter data to the radio
    - Downlink data via radio

- Dispatch() methods, each hardware driver (either a wrapper of an existing driver or a driver we develop)
    - Collect outputs from hardware 
    - Write to whiteboard fields 
    - Run any fault detection

- Radio Dispatch(), for radio dispatch we will want to just telemeter all available whiteboard channels. Need to determine how the radio wraps packets (if there is a built in driver for this then great, if not we will need to create a schema for packaging into UDP packets (this is really hard and we don't want to do this)). I would opt to let the ground station software handle everything afterwards, keep the on rocket software simple and let ground run a different stack (maybe python, js, etc, theres many built in packages for visualization that we can use)


