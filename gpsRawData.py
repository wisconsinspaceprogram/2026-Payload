import serial
import time
import csv
import math
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import threading
baseData = serial.Serial('com4',9600)         #rover test
time.sleep(1)

# Initialize the figure and axes for plotting
fig, axs = plt.subplots(3, 2, figsize=(10, 8))

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

# function to remove messy stuff at start
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

with open("gpsRawData.csv", mode="w", newline="") as file:
    writer = csv.writer(file)

    # Function to update the plots
    def update_plot(frame):
        # Check if there is new data to plot
        if len(time_elapsed) > 0:
            # Update each axis with the corresponding data
            axs[0, 0].cla()
            axs[0, 0].plot(time_elapsed, vg, label='Ground Velocity (km/h)')
            axs[0, 0].set_title('Ground Velocity (km/h)')
            axs[0, 0].set_xlabel('Time (s)')
            axs[0, 0].set_ylabel('Velocity (km/h)')

            axs[0, 1].cla()
            axs[0, 1].plot(time_elapsed, lat, label='Latitude')
            axs[0, 1].set_title('Latitude')
            axs[0, 1].set_xlabel('Time (s)')
            axs[0, 1].set_ylabel('Latitude')

            axs[1, 0].cla()
            axs[1, 0].plot(time_elapsed, long, label='Longitude')
            axs[1, 0].set_title('Longitude')
            axs[1, 0].set_xlabel('Time (s)')
            axs[1, 0].set_ylabel('Longitude')

            axs[1, 1].cla()
            axs[1, 1].plot(time_elapsed, alt, label='Altitude')
            axs[1, 1].set_title('Altitude (m)')
            axs[1, 1].set_xlabel('Time (s)')
            axs[1, 1].set_ylabel('Altitude (m)')

            axs[2, 0].cla()
            axs[2, 0].plot(time_elapsed, acc2d, label='HDOP')
            axs[2, 0].set_title('Horizontal Dilution of Precision (HDOP)')
            axs[2, 0].set_xlabel('Time (s)')
            axs[2, 0].set_ylabel('HDOP')

            axs[2, 1].cla()
            axs[2, 1].plot(time_elapsed, acc3d, label='VDOP')
            axs[2, 1].set_title('Vertical Dilution of Precision (VDOP)')
            axs[2, 1].set_xlabel('Time (s)')
            axs[2, 1].set_ylabel('VDOP')

        return axs

    def process_data():
        # receive data
        while True:
            # wait until there's data
            while baseData.inWaiting() == 0:
                pass

            # read one line at a time
            rdata = baseData.readline().decode('utf-8', errors = 'ignore').strip()
            dataPacket = remove_before_char(rdata, "$G")
            splitPacket = dataPacket.split(',')

            #print(str(baseData.readline()))
            #print(rdata)
            #print(dataPacket)
            #print(splitPacket)

            # dont process bad data
            if len(splitPacket) == 0:
                continue
            
            # message type
            con = str (splitPacket[0])

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
                        time_elapsed.append(0)
                    else:
                        time_diff = timeutc_hr * 3600 + timeutc_min * 60 + timeutc_sec - time_start
                        time_elapsed.append(time_diff)
                else:
                    time_ins = ""
                    time_elapsed.append(-1)
                
                '''print(f"Time Elapsed: {time_elapsed}")  # Debug line to check time elapsed'''
        
                # status
                if str (splitPacket[2]) == "A":
                    status = "Active, Valid"
                elif str (splitPacket[2]) == "V":
                    status = "Invalid"
                else:
                    status = ""

                # date
                if str (splitPacket[9]) != "":
                    rdate = str (splitPacket[9])
                    date = str (rdate[0:2]) + "-" + str (rdate[2:4]) + "-" + str (rdate[4:6])
                else:
                    date = ""

                # magnetic variation (degrees between true north and magnetic north at given location)
                if str (splitPacket[10]) and str (splitPacket[11]) != "":
                    magv = str (splitPacket[10])
                    magv_dir = str (splitPacket[11])
                else:
                    magv = "0"
                    magv_dir = ""

                writer.writerow(["time, status, date, magv, magv_dir"])
                writer.writerow([time_ins,status,date,magv,magv_dir])
                file.flush()
                #print(f"Time: {time_ins} {date} | Status: {status} | Magnetic Variation: {magv}° {magv_dir}")

            elif con == "$GNVTG":
                # true heading (heading relative to true north)
                if splitPacket[1] != "" and splitPacket[2] == "T":
                    thead = str (splitPacket[1])
                else:
                    thead = "0"
                
                # magnetic heading (heading relative to magnetic north)
                if splitPacket[3] != "" and splitPacket[4] == "M":
                    mhead = str (splitPacket[3])
                else:
                    mhead = "0"

                # ground velocity
                if splitPacket[5] != "" and splitPacket[6] == "N":
                    vg_knot = str (splitPacket[5])
                else:
                    vg_knot = ""
                if splitPacket[7] != "" and splitPacket[8] == "K":
                    vg_kmh = str (splitPacket[7])
                    vg.append(float(vg_kmh))
                else:
                    vg_kmh = ""
                    vg.append(-999)
                
                '''print(f"Appending Ground Velocity: {vg}")  # Debug line to check vg appending'''

                writer.writerow(["thead, mhead, vg_knot, vg_kmh"])
                writer.writerow([thead,mhead,vg_knot,vg_kmh])
                file.flush()
                #print(f"Heading - True: {thead}°, Magnetic: {mhead}° | Ground Velocity: {vg_knot} knots, {vg_kmh} km/h")

            elif con == "$GNGGA":
                # latitude
                if splitPacket[2] and splitPacket[3] != "":
                    rlat = str (splitPacket[2])
                    lat_ins = str (round(float(rlat[0:2]) + float(rlat[2:12]) / 60, 5))
                    lat.append(float(lat_ins))
                    lat_dir = str (splitPacket[3])
                else:
                    lat_ins = ""
                    lat.append(-999)
                    lat_dir = ""
                
                '''print(f"Appending Latitude: {lat}")  # Debug line to check lat appending'''

                # longitude
                if splitPacket[4] and splitPacket[5] != "":
                    rlong = str (splitPacket[4])
                    long_ins = str (round(float(rlong[0:3]) + float(rlong[3:13]) / 60, 5))
                    long.append(float(long_ins))
                    long_dir = str (splitPacket[5])
                else:
                    long_ins = ""
                    long.append(-999)
                    long_dir = ""
                
                '''print(f"Appending Longitude: {long}")  # Debug line to check long appending'''
                
                # total fix status
                if float(splitPacket[6]) == 0:
                    tfix = "No Fix"
                elif float(splitPacket[6]) == 1:
                    tfix = "2D GPS Fix"
                elif float(splitPacket[6]) == 2:
                    tfix = "3D GPS Fix"
                elif float(splitPacket[6]) == 4:
                    tfix = ""
                elif float(splitPacket[6]) == 5:
                    tfix = ""
                elif float(splitPacket[6]) == 6:
                    tfix = "Estimate"
                else:
                    tfix = ""

                # number of satellites
                if splitPacket[7] != "":
                    sat = str (int(splitPacket[7], 16))
                else:
                    sat = "0"

                # altitude
                if splitPacket[9] != "" and splitPacket[10] == "M":
                    alt_ins = str (splitPacket[9])
                    alt.append(float(alt_ins))
                else:
                    alt_ins = ""
                    alt.append(-999)
                
                '''print(f"Appending Altitude: {alt}")  # Debug line to check alt appending'''

                # geoid separation
                if splitPacket[11] != "" and splitPacket[12] == "M":
                    gsep = str (splitPacket[11])
                    alt_msl = str (round(float(splitPacket[9]) - float(splitPacket[11]), 1))
                else:
                    gsep = "0"
                    alt_msl = "0"

                writer.writerow(["lat, lat_dir, long, long_dir, tfix, sat, alt, gsep"])
                writer.writerow([lat_ins,lat_dir,long_ins,long_dir,tfix,sat,alt_ins,gsep])
                file.flush()
                #print(f"Position: {lat_ins}° {lat_dir}, {long_ins}° {long_dir} | Total Fix Mode: {tfix} | Number of Satellites: {sat} | Altitude: {alt_ins} m, {alt_msl} m msl")

            elif con == "$GNGSA":
                # satellite selection mode
                if splitPacket[1] == "A":
                    satmode = "Automatic"
                elif splitPacket[1] == "M":
                    satmode = "Manual"
                else:
                    satmode = ""
                
                # satellite IDs
                for i in range(3,15):
                    if splitPacket[i] == "":
                        satid = splitPacket[3:i]
                        break
                    else:
                        satid = splitPacket[3:15]

                # accuracy
                if splitPacket[16] != "" and splitPacket[16] != "99.99":
                    hdop = str (splitPacket[16])
                elif splitPacket[16] == "99.99":
                    hdop = ""
                else:
                    hdop = ""
                
                if len(acc2d) < len(time_elapsed):
                    acc2d.append(float(hdop))
                else:
                    pass
                '''print(f"Appending 2D Accuracy: {acc2d}")  # Debug line to check acc2d appending'''
                
                if splitPacket[17] != "" and splitPacket[17] != "99.99":
                    vdop = str (splitPacket[17])
                elif splitPacket[17] == "99.99":
                    vdop = ""
                else:
                    vdop = ""
                
                if splitPacket[15] != "" and splitPacket[15] != "99.99":
                    pdop = str (splitPacket[15])
                elif splitPacket[15] == "99.99":
                    pdop = ""
                else:
                    pdop = ""
                
                if len(acc3d) < len(time_elapsed):
                    acc3d.append(float(pdop))
                else:
                    pass
                '''print(f"Appending 3D Accuracy: {acc3d}")  # Debug line to check acc3d appending'''
                
                writer.writerow(["satmode, hdop, vdop, pdop"])
                writer.writerow([satmode, hdop, vdop, pdop])
                file.flush()
                '''if len(satid) != 0:
                    print(f"Satellite - Selection Mode: {satmode} | Satellite IDs: {satid} | Dilution of Precision - Horizontal: {hdop}, Vertical: {vdop}, Total: {pdop}")
                else:
                    pass'''

            elif con == "$GPGSV":
                # GPS satellites - total info (max 4 satellites per message)
                # number of messages
                if splitPacket[1] != "":
                    tm_gps = splitPacket[1]
                else:
                    tm_gps = ""
                # which message number
                if splitPacket[2] != "":
                    tmnum_gps = splitPacket[2]
                else:
                    tmnum_gps = ""
                # how many satellites total (all messages)
                if splitPacket[3] != "":
                    tsatnum_gps = int(splitPacket[3], 16)
                else:
                    tsatnum_gps = ""

                writer.writerow(["tm_gps, tmnum_gps, tsatnum_gps"])
                writer.writerow([tm_gps,tmnum_gps,tsatnum_gps])
                file.flush()
                '''if float(tmnum_gps) == 1:
                    print(f"Total # GPS Satellites: {tsatnum_gps}")
                else:
                    pass'''

                if tm_gps != tmnum_gps:
                    nsatnum_gps = 4
                else:
                    nsatnum_gps = int(min(10,tsatnum_gps)) - 4 * (int(tmnum_gps) - 1)
                
                if nsatnum_gps == 0:
                    pass
                else:
                    satids_gps = [splitPacket[i] for i in range(4, len(splitPacket), 4)]
                    # removing checksum
                    satid_gps = satids_gps[:-1]

                    satelev_gps = [splitPacket[i] for i in range(5, len(splitPacket), 4)]

                    satazi_gps = [splitPacket[i] for i in range(6, len(splitPacket), 4)]

                    satstr_gps = [splitPacket[i] for i in range(7, len(splitPacket), 4)]

                    writer.writerow(["nsatnum_gps, satid_gps, satelev_gps, satazi_gps, satstr_gps"])
                    writer.writerow([nsatnum_gps,satid_gps,satelev_gps,satazi_gps,satstr_gps])
                    file.flush()
                    '''for i in range(0,nsatnum_gps):
                        print(f"GPS Satellite {i+1} - ID: {satid_gps[i]}, Elevation: {satelev_gps[i]}°, Azimuth: {satazi_gps[i]}°, Strength: {satstr_gps[i]} dB")'''

            elif con == "$GLGSV":
                # GLONASS satellites
                if splitPacket[1] != "":
                    tm_gl = splitPacket[1]
                else:
                    tm_gl = ""

                if splitPacket[2] != "":
                    tmnum_gl = splitPacket[2]
                else:
                    tmnum_gl = ""

                if splitPacket[3] != "":
                    tsatnum_gl = int(splitPacket[3], 16)
                else:
                    tsatnum_gl = ""

                writer.writerow(["tm_gl, tmnum_gl, tsatnum_gl"])
                writer.writerow([tm_gl,tmnum_gl,tsatnum_gl])
                file.flush()
                '''if float(tmnum_gl) == 1:
                    print(f"Total # GLONASS Satellites: {tsatnum_gl}")
                else:
                    pass'''

                if tm_gl != tmnum_gl:
                    nsatnum_gl = 4
                else:
                    nsatnum_gl = int(min(10,tsatnum_gl)) - 4 * (int(tmnum_gl) - 1)
                
                if nsatnum_gl == 0:
                    pass
                else:
                    satids_gl = [splitPacket[i] for i in range(4, len(splitPacket), 4)]
                    satid_gl = satids_gl[:-1]

                    satelev_gl = [splitPacket[i] for i in range(5, len(splitPacket), 4)]

                    satazi_gl = [splitPacket[i] for i in range(6, len(splitPacket), 4)]

                    satstr_gl = [splitPacket[i] for i in range(7, len(splitPacket), 4)]

                    writer.writerow(["nsatnum_gl, satid_gl, satelev_gl, satazi_gl, satstr_gl"])
                    writer.writerow([nsatnum_gl,satid_gl,satelev_gl,satazi_gl,satstr_gl])
                    file.flush()
                    '''for i in range(0,nsatnum_gl):
                        print(f"GLONASS Satellite {i+1} - ID: {satid_gl[i]}, Elevation: {satelev_gl[i]}°, Azimuth: {satazi_gl[i]}°, Strength: {satstr_gl[i]} dB")'''

            elif con == "$GAGSV":
                # Galileo satellites
                if splitPacket[1] != "":
                    tm_ga = splitPacket[1]
                else:
                    tm_ga = ""

                if splitPacket[2] != "":
                    tmnum_ga = splitPacket[2]
                else:
                    tmnum_ga = ""

                if splitPacket[3] != "":
                    tsatnum_ga = int(splitPacket[3], 16)
                else:
                    tsatnum_ga = ""

                writer.writerow(["tm_ga, tmnum_ga, tsatnum_ga"])
                writer.writerow([tm_ga,tmnum_ga,tsatnum_ga])
                file.flush()
                '''if float(tmnum_ga) == 1:
                    print(f"Total # Galileo Satellites: {tsatnum_ga}")
                else:
                    pass'''

                if tm_ga != tmnum_ga:
                    nsatnum_ga = 4
                else:
                    nsatnum_ga = int(min(10,tsatnum_ga)) - 4 * (int(tmnum_ga) - 1)
                
                if nsatnum_ga == 0:
                    pass
                else:
                    satids_ga = [splitPacket[i] for i in range(4, len(splitPacket), 4)]
                    satid_ga = satids_ga[:-1]

                    satelev_ga = [splitPacket[i] for i in range(5, len(splitPacket), 4)]

                    satazi_ga = [splitPacket[i] for i in range(6, len(splitPacket), 4)]

                    satstr_ga = [splitPacket[i] for i in range(7, len(splitPacket), 4)]

                    writer.writerow(["nsatnum_ga, satid_ga, satelev_ga, satazi_ga, satstr_ga"])
                    writer.writerow([nsatnum_ga,satid_ga,satelev_ga,satazi_ga,satstr_ga])
                    file.flush()
                    '''for i in range(0,nsatnum_ga):
                        print(f"Galileo Satellite {i+1} - ID: {satid_ga[i]}, Elevation: {satelev_ga[i]}°, Azimuth: {satazi_ga[i]}°, Strength: {satstr_ga[i]} dB")'''

            elif con == "$GBGSV":
                # BeiDou satellites
                if splitPacket[1] != "":
                    tm_bd = splitPacket[1]
                else:
                    tm_bd = ""

                if splitPacket[2] != "":
                    tmnum_bd = splitPacket[2]
                else:
                    tmnum_bd = ""

                if splitPacket[3] != "":
                    tsatnum_bd = int(splitPacket[3], 16)
                else:
                    tsatnum_bd = ""

                writer.writerow(["tm_bd, tmnum_bd, tsatnum_bd"])
                writer.writerow([tm_bd,tmnum_bd,tsatnum_bd])
                file.flush()
                '''if float(tmnum_bd) == 1:
                    print(f"Total # BeiDou Satellites: {tsatnum_bd}")
                else:
                    pass'''

                if tm_bd != tmnum_bd:
                    nsatnum_bd = 4
                else:
                    nsatnum_bd = int(min(10,tsatnum_bd)) - 4 * (int(tmnum_bd) - 1)
                
                if nsatnum_bd == 0:
                    pass
                else:
                    satids_bd = [splitPacket[i] for i in range(4, len(splitPacket), 4)]
                    satid_bd = satids_bd[:-1]

                    satelev_bd = [splitPacket[i] for i in range(5, len(splitPacket), 4)]

                    satazi_bd = [splitPacket[i] for i in range(6, len(splitPacket), 4)]

                    satstr_bd = [splitPacket[i] for i in range(7, len(splitPacket), 4)]

                    writer.writerow(["nsatnum_bd, satid_bd, satelev_bd, satazi_bd, satstr_bd"])
                    writer.writerow([nsatnum_bd,satid_bd,satelev_bd,satazi_bd,satstr_bd])
                    file.flush()
                    '''for i in range(0,nsatnum_bd):
                        print(f"BeiDou Satellite {i+1} - ID: {satid_bd[i]}, Elevation: {satelev_bd[i]}°, Azimuth: {satazi_bd[i]}°, Strength: {satstr_bd[i]} dB")'''
            
            elif con == "$GQGSV":
                # Michibiki satellites - total info
                if splitPacket[1] != "":
                    tm_qz = splitPacket[1]
                else:
                    tm_qz = ""

                if splitPacket[2] != "":
                    tmnum_qz = splitPacket[2]
                else:
                    tmnum_qz = ""

                if splitPacket[3] != "":
                    tsatnum_qz = int(splitPacket[3], 16)
                else:
                    tsatnum_qz = ""

                writer.writerow(["tm_qz, tmnum_qz, tsatnum_qz"])
                writer.writerow([tm_qz,tmnum_qz,tsatnum_qz])
                file.flush()
                '''if float(tmnum_qz) == 1:
                    print(f"Total # Michibiki Satellites: {tsatnum_qz}")
                else:
                    pass'''

                if tm_qz != tmnum_qz:
                    nsatnum_qz = 4
                else:
                    nsatnum_qz = int(min(10,tsatnum_qz)) - 4 * (int(tmnum_qz) - 1)
                
                if nsatnum_qz == 0:
                    pass
                else:
                    satids_qz = [splitPacket[i] for i in range(4, len(splitPacket), 4)]
                    satid_qz = satids_qz[:-1]

                    satelev_qz = [splitPacket[i] for i in range(5, len(splitPacket), 4)]

                    satazi_qz = [splitPacket[i] for i in range(6, len(splitPacket), 4)]

                    satstr_qz = [splitPacket[i] for i in range(7, len(splitPacket), 4)]

                    writer.writerow(["nsatnum_qz, satid_qz, satelev_qz, satazi_qz, satstr_qz"])
                    writer.writerow([nsatnum_qz,satid_qz,satelev_qz,satazi_qz,satstr_qz])
                    file.flush()
                    '''for i in range(0,nsatnum_qz):
                        print(f"Michibiki Satellite {i+1} - ID: {satid_qz[i]}, Elevation: {satelev_qz[i]}°, Azimuth: {satazi_qz[i]}°, Strength: {satstr_qz[i]} dB")'''
            
            else:
                pass

            # Update the plot
            if len(time_elapsed) == len(acc3d):
                update_plot(None)
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
    plt.show()

# Start data processing and plotting
start_data_processing()
run_animation()