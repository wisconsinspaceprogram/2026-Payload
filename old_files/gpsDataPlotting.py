import serial
import time
import csv
import math
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import threading

# Initialize the serial port (your communication setup)
baseData = serial.Serial('com4', 9600)  # Example for rover test
time.sleep(1)

# Initialize the figure and axes for plotting
fig, axs = plt.subplots(3, 2, figsize=(12, 7))

# Define the plot titles and labels
axs[0, 0].set_title('Ground Velocity (km/h)')
axs[0, 0].set_xlabel('Time (s)')
axs[0, 0].set_ylabel('Velocity (km/h)')

axs[0, 1].set_title('Latitude')
axs[0, 1].set_xlabel('Time (s)')
axs[0, 1].set_ylabel('Latitude')

axs[1, 0].set_title('Longitude')
axs[1, 0].set_xlabel('Time (s)')
axs[1, 0].set_ylabel('Longitude')

axs[1, 1].set_title('Altitude (m)')
axs[1, 1].set_xlabel('Time (s)')
axs[1, 1].set_ylabel('Altitude (m)')

axs[2, 0].set_title('Horizontal Dilution of Precision (HDOP)')
axs[2, 0].set_xlabel('Time (s)')
axs[2, 0].set_ylabel('HDOP')

axs[2, 1].set_title('Vertical Dilution of Precision (VDOP)')
axs[2, 1].set_xlabel('Time (s)')
axs[2, 1].set_ylabel('VDOP')

# Initialize lists for storing the data
time_elapsed = []
vg = []
lat = []
long = []
alt = []
acc2d = []
acc3d = []

# Function to remove messy stuff at start
def remove_before_char(text, char):
    index = text.find(char)
    if index != -1:
        return text[index:]
    else:
        return ""

# or at the end
def keep_before_char(text, char):
    index = text.find(char)
    if index != -1:
        return text[:index]
    else:
        return ""

# Function to update the plots
def update_plot(frame):
    if len(time_elapsed) > 0:
        # Plot data
        axs[0, 0].plot(time_elapsed, vg, label='Ground Velocity (km/h)', color='blue')
        axs[0, 1].plot(time_elapsed, lat, label='Latitude', color='red')
        axs[1, 0].plot(time_elapsed, long, label='Longitude', color='green')
        axs[1, 1].plot(time_elapsed, alt, label='Altitude (m)', color='purple')
        axs[2, 0].plot(time_elapsed, acc2d, label='HDOP', color='black')
        axs[2, 1].plot(time_elapsed, acc3d, label='VDOP', color='black')

        # Dynamically adjust the y-axis limits for each plot
        axs[0, 0].set_ylim([min(vg) - 0.1, max(vg) + 0.1])  # Ground Velocity
        axs[0, 1].set_ylim([min(lat) - 0.001, max(lat) + 0.001])  # Latitude
        axs[1, 0].set_ylim([min(long) - 0.001, max(long) + 0.001])  # Longitude
        axs[1, 1].set_ylim([min(alt) - 1, max(alt) + 1])  # Altitude
        axs[2, 0].set_ylim([min(acc2d) - 0.5, max(acc2d) + 0.5])  # HDOP
        axs[2, 1].set_ylim([min(acc3d) - 0.5, max(acc3d) + 0.5])  # VDOP

        # Update titles and labels outside the loop to avoid redundancy
        for ax in axs.flat:
            ax.set_xlabel('Time (s)')
            ax.set_ylabel('Value')

    return axs

data_lock = threading.Lock()
# Data processing function
def process_data():
    with data_lock:
        # Open file once outside the loop
        with open("gpsRawData.csv", mode="w", newline="") as file:
            writer = csv.writer(file)

            # Write headers only once
            writer.writerow(["time_elapsed", "vg", "lat", "long", "alt", "acc2d", "acc3d"])

            start_time = time.time()  # Track the start time

            while True:
                # Wait until there's data available
                while baseData.inWaiting() == 0:
                    pass

                # Read one line at a time
                rdata = baseData.readline().decode('utf-8', errors='ignore').strip()
                dataPacket = remove_before_char(rdata, "$G")
                splitPacket = dataPacket.split(',')

                if len(splitPacket) == 0:
                    continue

                con = str(splitPacket[0])

                # Processing different GPS messages like $GNRMC, $GNVTG, etc.
                time_diff = 0
                # can't loop using system time, as it appends for each line that comes in
                if con == "$GNRMC":
                    # global time (adjusted from GMT)
                    if splitPacket[1] != "":                     
                        timeutc = str (splitPacket[1])
                        timeutc_hr  = (float(timeutc[0:2]) - 5) % 24
                        timeutc_min = float(timeutc[2:4])
                        timeutc_sec = float(timeutc[4:6])
                        time_ins = str (round(float(timeutc[0:2]) - 5) % 24) + ":" + str (timeutc[2:4]) + ":" + str (timeutc[4:6])
                        if len(time_elapsed) == 0:
                            time_start = timeutc_hr * 3600 + timeutc_min * 60 + timeutc_sec
                            time_diff = 0
                            time_elapsed.append(0)
                        else:
                            time_diff = timeutc_hr * 3600 + timeutc_min * 60 + timeutc_sec - time_start
                            time_elapsed.append(time_diff)
                    else:
                        time_ins = ""
                        time_diff = ""
                        time_elapsed.append(time_elapsed[-1] + 1)

                if con == "$GNVTG":
                    if splitPacket[7] != "" and splitPacket[8] == "K":
                        vg_kmh = str(splitPacket[7])
                        vg.append(float(vg_kmh))
                    else:
                        vg_kmh = ""
                        vg.append(-1)

                elif con == "$GNGGA":
                    if splitPacket[2] and splitPacket[3] != "":
                        rlat = str(splitPacket[2])
                        lat_deg = float(rlat[0:2])
                        lat_min = float(rlat[2:12])
                        lat_ins = lat_deg + (lat_min / 60)

                        # Apply hemisphere correction
                        if splitPacket[3] == "S":
                            lat_ins = -lat_ins

                        lat.append(round(lat_ins, 5))
                    else:
                        lat_ins = ""
                        lat.append(0)

                    if splitPacket[4] and splitPacket[5] != "":
                        rlong = str(splitPacket[4])
                        long_deg = float(rlong[0:3])
                        long_min = float(rlong[3:13])
                        long_ins = long_deg + (long_min / 60)

                        # Apply hemisphere correction
                        if splitPacket[5] == "W":
                            long_ins = -long_ins

                        long.append(round(long_ins, 5))
                    else:
                        long_ins = ""
                        long.append(0)

                    if splitPacket[9] != "" and splitPacket[10] == "M":
                        alt_ins = str(splitPacket[9])
                        alt.append(float(alt_ins))
                    else:
                        alt_ins = ""
                        alt.append(0)

                elif con == "$GNGSA":
                    if splitPacket[16] != "" and splitPacket[16] != "99.99":
                        hdop = str(splitPacket[16])
                    elif splitPacket[16] == "99.99":
                        hdop = str(-1)
                    else:
                        hdop = str(-1)

                    if len(acc2d) < len(time_elapsed):
                        acc2d.append(float(hdop))
                    else:
                        pass

                    if splitPacket[15] != "" and splitPacket[15] != "99.99":
                        pdop = str (splitPacket[15])
                    elif splitPacket[15] == "99.99":
                        pdop = str(-1)
                    else:
                        pdop = str(-1)
                    
                    if len(acc3d) < len(time_elapsed):
                        acc3d.append(float(pdop))
                    else:
                        pass

                # Update the plot
                if len(vg) or len(lat) or len(long) or len(alt) or len(acc2d) or len(acc3d) == 0:
                    pass
                else:
                    if len(time_elapsed) == len(vg) == len(lat) == len(long) == len(alt) == len(acc2d) == len(acc3d):
                        update_plot(None)
                        # Write data to CSV every time new data is processed
                        writer.writerow([time_diff, vg[-1], lat[-1], long[-1], alt[-1], acc2d[-1], acc3d[-1]])
                        file.flush()
                    else:
                        pass

# Start the process in a separate thread
def start_data_processing():
    data_thread = threading.Thread(target=process_data)
    data_thread.daemon = True
    data_thread.start()

# Run the plot animation
def run_animation():
    ani = FuncAnimation(fig, update_plot, interval=1000, blit=False, cache_frame_data=False)
    plt.tight_layout()
    plt.subplots_adjust(left=0.05, right=0.95, top=0.95, bottom=0.05, wspace=0.2, hspace=0.3)
    '''manager = plt.get_current_fig_manager()
    manager.window.state('zoomed')'''
    plt.show()

# Start data processing and plotting
start_data_processing()
run_animation()