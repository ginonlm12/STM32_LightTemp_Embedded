# STM32 Light and Temperature Control Embedded System

This project implements a simple embedded system using STM32 microcontrollers to manage light and temperature control based on user-defined settings or automatic modes. The project is designed using **STM32CubeIDE** and **TouchGFX** for GUI development.

## Features

- **Manual and Automatic Modes**:
  - **Manual Mode**: Users can toggle the light and set temperature limits manually.
  - **Automatic Mode**: Light and fan control are automatically adjusted based on sensor readings and predefined thresholds.
- Real-time sensor data display.
- Responsive graphical user interface (GUI) using TouchGFX.
- User-friendly control buttons for increasing/decreasing thresholds and toggling modes.

## How to Use

### 1. Update the `.project` Path
1. Navigate to the file: `STM32_LightTemp_Embedded/STM32CubeIDE/.project`.
2. Open the file in a text editor.
3. Update any absolute paths to match your local directory where the `STM32_LightTemp_Embedded` folder resides.

### 2. Open in STM32CubeIDE
1. Launch **STM32CubeIDE**.
2. Select your workspace directory.

### 3. Import the Project
1. Go to **File > Import**.
2. Select **Existing Projects into Workspace**.
3. Browse to the directory containing the repository.
4. Ensure the project is detected and click **Finish**.

### 4. Build and Flash
1. Build the project in **STM32CubeIDE**.
2. Connect your STM32 development board to your computer.
3. Flash the firmware onto the board and monitor its functionality.

## Project Structure

- **`STM32_LightTemp_Embedded/`**:
  - Contains the STM32CubeIDE project files and source code.
  - GUI development files are located in the `TouchGFX/` folder.
  - Core embedded logic resides in the `Core/` folder.

## License and Credits

This project is open-source and free to use for educational and personal purposes. Attribution to the original contributors is appreciated if reused or modified.

### Contributors

- [Hoang Lam Nguyen](https://github.com/ginonlm12)
- [Thanh Lap Pham](https://github.com/EnmutsubiKami)

Feel free to fork this repository and contribute to the project by submitting pull requests!
