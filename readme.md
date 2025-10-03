# mini-fzf-fast

A **lightweight, fast, interactive file and directory finder** for Linux and WSL.
Built in C using **ncurses**, it allows instant filtering of files and directories directly from the terminal.

---

## Features

- Interactive search with live filtering as you type.
- Matches both **basename** and full path of files and directories.
- Supports **arrow key navigation** and **Enter** to select.
- Color-coded display:

  - **Blue**: directories
  - **Green**: executables
  - **White**: regular files

- Keyboard shortcuts:

  - `↑` / `↓` : move selection
  - `Enter` : select file/directory
  - `ESC` / `ALT+Q` : quit

---

## Installation

This project includes a simple installation script (`install.sh`) that builds and installs `mini-fzf-fast` system-wide and handles dependencies.

1. Clone the repository:

```bash
git clone https://git@github.com:Devesh-Jaiswal/mini_fzf_fast.git
cd mini-fzf-fast
```

2. Make the install script executable and run it:

```bash
chmod +x install.sh
./install.sh
```

> The script will automatically install required dependencies (`gcc`, `ncurses`, `pkg-config`), build the program, and install it to `/usr/local/bin`.

---

## Usage

Run the program with the directory you want to search:

```bash
mini_fzf_fast /path/to/search
```

- Start typing to filter files and directories.
- Navigate results using arrow keys.
- Press `Enter` to select a file/directory (prints the path to stdout).
- Press `ESC` or `Ctrl+Q` to exit without selection.

**Example:**

```bash
mini_fzf_fast ~/projects
```

---

## Notes

- Only supports **Linux and WSL** (not native Windows).
- Requires **ncurses**.
- Lightweight and fast — single-file C program with minimal dependencies.

---

## License
