
# 🌱 VF Device: Your Personal Vertical Farming Assistant 🚀

**VF Device** is an IoT device designed to help people establish a vertical farm in their homes. Built using the ESP32 Node MCU development kit and PlatformIO, this device makes vertical farming accessible and manageable for everyone. The project leverages Google Firebase real-time database to store crop values and take necessary actions based on the data.

## 🚀 Key Features

- **IoT-Enabled**: Manage and monitor your vertical farm remotely.
- **Real-Time Data Storage**: Uses Google Firebase for storing and retrieving crop values in real-time.
- **Automated Actions**: Takes actions based on real-time data to ensure optimal growing conditions.
- **User-Friendly Interface**: Easy to set up and use, making vertical farming accessible to everyone.

## 🛠️ Toolkits

VF Device is built using the following toolkits and technologies:

- **ESP32 Node MCU**: Core hardware for the IoT device.
- **PlatformIO**: Integrated development environment for IoT development.
- **Google Firebase**: Real-time database for storing crop values and controlling actions.
- **Sensors & Actuators**: Various sensors to monitor environmental conditions and actuators to maintain optimal conditions.

## 🖥️ User Interface

The VF Device comes with a user-friendly interface to manage and monitor your vertical farm:

- **Real-Time Monitoring**: View live data from your vertical farm.
- **Automated Controls**: Set thresholds and automate actions based on sensor data.

## 🛠️ Installation

Follow these steps to get your VF Device up and running:

1. **Clone the Repository**:
    ```sh
    git clone https://github.com/yourusername/vf-device.git
    cd vf-device
    ```

2. **Install Dependencies**:
    ```sh
    # Install PlatformIO Core
    pip install platformio

    # Install the project dependency and open the platfomrIO files
    # first, navigate to the repo directory
    pio pkg install
    ```

3. **Configure Firebase**:
    - Set up a Firebase project on Google Firebase.
    - Replace the placeholders in `API_KEY` and `RTDB_URL` with your Firebase configs.

4. **Upload Code to ESP32**:
    ```sh
    pio run --target upload
    ```

5. **Monitor Serial Output**:
    ```sh
    pio device monitor
    ```

## 💬 Contact

For questions or comments, feel free to reach out to us at [omardwahba71@gmail.com](mailto:omardwahba71@gmail.com).

---

*Happy Farming!* 🌱✨

---
