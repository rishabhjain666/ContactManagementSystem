# ContactHub — C Contact Management Web App

A beginner-friendly local web application for the DSA Contact Management System assignment. The frontend uses HTML, CSS and JavaScript. The backend and data structures are written in C. It uses Windows Winsock for HTTP, so no Python virtual environment, Node.js, database, or third-party C library is needed.

## Features

- Add, edit/update and delete contacts
- Search exact contact names using a binary search tree
- Search phone numbers using a hash table with separate chaining for collisions
- Display all contacts using a linked list
- Alphabetical sorting using Bubble Sort
- Group/filter contacts by category (Family, Friend, Work, Other)
- Responsive browser interface
- Duplicate ID and phone checks

## Requirements

- Windows 10/11
- GCC for Windows (MSYS2 UCRT64 recommended)
- A modern browser

## Install GCC (one time)

1. Install MSYS2 from https://www.msys2.org/ using the default location.
2. Open **MSYS2 UCRT64** from the Start menu.
3. Run:

   ```bash
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc
   ```

4. Add `C:\msys64\ucrt64\bin` to your Windows user `Path` environment variable.
5. Close and reopen VS Code/PowerShell. Check with `gcc --version`.

## Run the app

1. Extract the ZIP to a normal folder, for example `C:\project\ContactManagementSystem`.
2. Double-click `build.bat` and wait for `Build successful`.
3. Double-click `start.bat` (or run `contact_server.exe` from the project root).
4. Open http://127.0.0.1:8080 in your browser.
5. Keep the terminal window open while using the app. Press Ctrl+C in it to stop the server.

Alternatively, from PowerShell in the project folder:

```powershell
gcc -std=c11 backend\main.c -o contact_server.exe -lws2_32
.\contact_server.exe
```

Then open http://127.0.0.1:8080.

## Project structure

```text
ContactManagementSystem/
├── frontend/
│   ├── index.html       # Page structure and contact form
│   ├── style.css        # Responsive styling
│   └── app.js           # Browser UI and HTTP requests
├── backend/
│   └── main.c           # C HTTP server, linked list, hash table, BST, Bubble Sort
├── build.bat            # Compiles the C server
├── start.bat            # Starts the server
└── README.md            # Setup and usage instructions
```

## Notes / limitations

- Contacts are stored in memory. They reset when the server is stopped; persistent storage is not included in this starter project.
- This is a local educational demo, not a production server. Do not expose it directly to the public internet.
- Search by name is an exact, case-insensitive match. The frontend quick-search filters the loaded contact list by name, phone or ID.
- The BST is rebuilt after each change to keep indexes synchronized. This is easy to explain for a DSA demonstration, though not optimized for large contact lists.
- The server serves the frontend from the project root. Start it while your working directory is the extracted project folder, as the batch file does.
