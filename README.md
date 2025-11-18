# UniVend – Integrated Vending Machine (Arduino Mega)

UniVend is a prototype vending machine that offers hot coffee, cold juice, and filtered hot or cold water. It runs on a compact automated system powered by an Arduino Mega.
We created UniVend as our capstone project at the Polytechnic University of the Philippines, Santa Maria Campus. Our aim is to save space, make vending more convenient, and modernize traditional vending machines.

---

## 🔧 Technology Stack

- **Arduino Mega 2560**  
- **Arduino IDE** (Firmware development)  
- **C/C++** (Arduino programming)  
- **Electrical & mechanical prototyping**  
- **Reverse osmosis water filtration**  
- **Sensors & actuators integration**

---

## 🚀 Features

### ✔️ Multi-beverage dispensing  
- Hot coffee  
- Cold juice  
- Hot filtered water  
- Cold filtered water  
- All controlled using a single Arduino Mega system

### ✔️ Automated Water Refilling System  
- Uses **6-stage reverse osmosis filtration**  
- Monitors top & bottom water tanks  
- Automatically activates pump/valve when water is low

### ✔️ Smart Payment System  
- Calibrated coin slot  
- Accepts only **₱5 coins** (old & new)  
- Rejects excess or early coin insertion  
- Ensures correct payment before dispensing

### ✔️ User-Friendly Interface  
- LCD screen with clear prompts  
- Left/Right button navigation  
- 30-second timeout auto-reset  
- Voice guidance + background audio

### ✔️ Inventory Monitoring  
- IR sensors detect low powder levels  
- Coin box capacity detection  
- Sends **SMS alerts** to the owner via GSM module

### ✔️ Security & Maintenance  
- External locks to protect internal components  
- Self-cleaning mode activated by an internal switch
- Removable powder canister, funnel, and short hose  
- Easy manual cleaning

---

## 🧠 System Overview

UniVend brings together several subsystems:

- **Arduino Mega 2560** (main controller)  
- LCD + buttons (user interface)  
- Coin slot module  
- Servo & mixing motors  
- 12V pumps and solenoid valves  
- 6-Stage RO water filter  
- IR sensors for inventory  
- GSM module for alerts  
- Audio module & speaker  
- External lock system  

This modular design makes it easier to upgrade, improve, and add new features in the future.

