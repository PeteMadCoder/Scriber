# Project Roadmap & TODOs

## Refactoring & Architecture
- [x] **Restructure Project Layout**: Move source code into `src/`, headers into `include/`, and resources into `resources/` (standard C++ structure). Ensure CMake build system is updated to reflect these changes.

## Core Features & Fixes
- [x] **Fix Unsaved Changes Indicator**: 
    - Add a `*` suffix to the window title/tab when the file has unsaved changes (dirty state).
    - Remove the `*` immediately upon saving.
    - Fix the "Save Changes?" popup logic: only show it if the file is truly "dirty" (unsaved modification).
- [x] **Pitch Black Theme**:
    - Add a new theme with `#000000` background and off-white/grey text (high contrast, OLED friendly).
- [x] **Portuguese Localization**:
    - Add Portuguese (pt-PT/pt-BR) to the spellchecker supported languages.
    - (Optional) Translate UI strings if applicable.

## Advanced Integration
- [x] **Integrate `cmark`**:
    - [x] Use `cmark` for HTML/PDF export.
    - [x] Replace or augment the current Regex-based Markdown parsing with `cmark` (CommonMark) for better accuracy and performance (Implemented Document Outline).
    - [x] Use the AST for cleaner export to HTML/PDF (Enabled Smart Punctuation and UTF-8 validation).
- [x] **Sidebar File Explorer**:
    - Implement a toggleable sidebar showing a file tree view of the current directory (similar to xed).
- [x] **Multi-language Code Block Highlighting**:
    - Implement syntax highlighting for code blocks within Markdown files (e.g., Python, C++, Bash).
    - Consider using `KSyntaxHighlighting` or `cmark`'s AST for language detection.

## Performance
- [x] **Large File Optimization**:
    - Implement debouncing for syntax highlighting and spell checking to prevent UI lag on large documents.
    - Investigate `QPlainTextEdit` performance bottlenecks.

## UI/UX Enhancements (Planned)
- [x] **Multiple File Support (Tabs)**:
    - Implement a tabbed interface (using `QTabWidget`) to allow opening and editing multiple files simultaneously.
- [ ] **Enhanced Sidebar Navigation**:
    - Improve sidebar to mimic xed's file browser.
    - Add "Go Up / Parent Directory" button.
    - Show current directory path.
    - Allow basic file management (create, delete, rename).
- [ ] **Global Application Theming**:
    - Extend themes (Dark, Pitch Black) to style the entire application UI (menus, status bar, sidebar), not just the editor widget.
    - Replace "Toggle Theme" with a "Select Theme" menu or dialog for explicit choice.

## Under Consideration (Not Priority)
*These features are complex and currently under review. Implementation is not guaranteed.*

- [ ] **Inline Images**:
    - **Concept**: Display actual images within the editor view instead of just Markdown syntax.
    - **Hurdles**: `QPlainTextEdit` is optimized for text, not rich media. Switching to `QTextEdit` might sacrifice performance for large files. Custom rendering requires significant low-level work.
    
- [ ] **Mermaid Graphs**:
    - **Concept**: Render Mermaid diagrams directly within the editor.
    - **Hurdles**: Mermaid is a JavaScript library, requiring a web engine (Chromium) to render. Embedding a web view is resource-heavy and complex. Static image generation (via Node.js) is feasible but not interactive.
