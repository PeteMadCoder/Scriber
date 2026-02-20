# Scriber - Distraction-Free Markdown Editor

![Scriber Screenshot](screenshot.png)

Scriber is a distraction-free Markdown editor built with Qt that provides a clean, focused writing experience with real-time syntax highlighting. Unlike traditional Markdown editors that require separate preview panes, Scriber renders your Markdown directly in the editor with proper formatting while maintaining the underlying Markdown syntax.

---

## Features

**Real-time Markdown Rendering**: See formatted text as you type, with proper bold, italic, headings, and other Markdown elements
**Distraction-Free Interface**: Clean UI that keeps your focus on writing
**Zoom Support**: `Ctrl+Mouse Wheel` or `Ctrl+'+'/'-'` to adjust text size
**Light/Dark Themes**: Toggle between themes with a single click
**Syntax Highlighting**: Markdown syntax characters are visually deemphasized while content is properly formatted
**Find Functionality**: Quick find (`Ctrl+F`) with an embedded search bar at the bottom of the window.
**Cross-Platform**: Works on Windows, macOS, and Linux

---

## Installation

### Linux

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/scriber.git
   cd scriber
   ```

2. Build the application:
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

3. Install the application (requires sudo for system-wide installation):
   ```bash
   # For system-wide installation (all users)
   sudo ../install.sh
   
   # For user-only installation
   ../install.sh
   ```

### Windows and macOS

Windows and macOS builds are coming soon. For now, you can build from source:

1. Clone the repository
2. Create a build directory and run CMake
3. Build the project using your preferred IDE or build system

---

## Usage

**Basic Editing**: Type Markdown as usual - Scriber will format it in real-time
**Zoom Controls**:
- Ctrl + Mouse Wheel Up/Down: Zoom in/out
- Ctrl + '+': Zoom in
- Ctrl + '-': Zoom out
- Ctrl + '0': Reset zoom
**Theme Toggle**: Press F12 or use the View menu to toggle between light and dark themes
**Find Text**: Press `Ctrl + F` to open the embedded find bar at the bottom.
**Spell Check**: Enabled by default. Toggle via `Tools -> Spell Check`. Change language via `Tools -> Language`.
**Markdown Shortcuts**:
 - Ctrl + B: Insert bold text (`**text**`)
 - Ctrl + I: Insert italic text (`*text*`)
 - Ctrl + K: Insert inline code (`` `code` ``)
 - Ctrl + L: Insert link (`[text](url)`)

## Building from Source

1. Install dependencies:
   - **Linux**: `sudo apt install cmake qt6-base-dev qt6-tools-dev build-essential`
   - **macOS**: Install Xcode Command Line Tools and Qt 6 via Homebrew
   - **Windows**: Install Visual Studio, CMake, and Qt 6

2. Build the project:
   ```bash
   git clone https://github.com/yourusername/scriber.git
   cd scriber
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

3. Run the application:
   ```bash
   ./scriber
   ```

---

## Architecture & Implementation Notes

Scriber is built using the Qt 6 framework, primarily leveraging `QPlainTextEdit` for the editing surface and `QSyntaxHighlighter` for real-time formatting.

- **Core Editor (`EditorWidget`)**:
  - Inherits from `QPlainTextEdit` for efficient handling of large plain text documents.
  - Uses a custom `MarkdownHighlighter` to apply visual formatting (bold, italic, colors) based on Markdown syntax.
  - Integrates Hunspell-based `SpellChecker` for real-time spell checking.
  - Manages zoom functionality by adjusting the document's default font size and re-highlighting.

- **Markdown Highlighting (`MarkdownHighlighter`)**:
  - Subclasses `QSyntaxHighlighter`.
  - Defines rules using `QRegularExpression` to identify Markdown elements.
  - Applies `QTextCharFormat` (font weight, style, color) to the *content* of Markdown elements.
  - Makes Markdown syntax characters (e.g., `**`, `#`) visually faint by applying a format with a color matching the background.
  - Uses `QTextBlockFormat` (margins, line height, background) for block-level elements like headings and code blocks.
  - Implements light and dark themes with distinct color palettes.
  - Follows the Qt documentation approach where "formatting properties are merged at display time".

- **Spell Checking (`SpellChecker`)**:
  - A Qt wrapper around the Hunspell library.
  - Handles loading dictionaries, checking words, and suggesting corrections.
  - Uses UTF-8 encoding for compatibility with most dictionaries.

- **Find Functionality (`MainWindow`)**:
  - Implements an embedded find bar at the bottom of the main window instead of a popup dialog.
  - Uses `QTextEdit::find()` for efficient text searching with options (case-sensitive, whole words).

- **File Handling (`FileManager`)**:
  - Manages loading and saving Markdown files.
  - Handles export to HTML and PDF using `cmark` for CommonMark-compliant rendering.

- **Document Outline (`DocumentOutlineWidget`)**:
  - Uses `cmark` AST to parse and display document structure.
  - Provides clickable navigation to headings with proper nesting.

- **Sidebar File Explorer (`SidebarFileExplorer`)**:
  - Toggleable file tree showing current directory structure.
  - Supports file management: create, delete, rename, duplicate, copy/paste.

---

## Development Roadmap

For the complete list of planned features and improvements, see [TODO.md](TODO.md).

Key areas of focus:
- **Markdown Rendering**: Improved parsing robustness, better table support, true horizontal rules
- **UI/UX**: Find & replace, recent files, session management, font customization
- **Theme System**: Secondary/accent color support, customizable themes
- **Packaging**: Linux packages (.deb, .rpm, AppImage), Windows/macOS installers
- **Code Quality**: Unit tests, documentation, refactoring

---