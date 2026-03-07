# Project Roadmap & TODOs

## Completed Tasks

### Refactoring & Architecture
- [x] **Restructure Project Layout**: Move source code into `src/`, headers into `include/`, and resources into `resources/` (standard C++ structure).
- [x] **Multiple File Support (Tabs)**: Tabbed interface using `QTabWidget`.
- [x] **Global Application Theming**: System-wide themes (Light, Dark, Pitch Black) via `ThemeManager` and `ThemeDialog`.

### Core Features & Fixes
- [x] **Fix Unsaved Changes Indicator**: `*` preffix in window title/tab for dirty state.
- [x] **Pitch Black Theme**: Pure black background, high contrast for OLED displays.
- [x] **Portuguese Localization**: pt-PT/pt-BR added to spellchecker languages.

### Advanced Integration
- [x] **Integrate `cmark`**: CommonMark-compliant parsing for export and document outline.
- [x] **Sidebar File Explorer**: Toggleable file tree with management capabilities.
- [x] **Multi-language Code Block Highlighting**: Python, C++, Bash syntax highlighting in fenced code blocks.

### Performance
- [x] **Large File Optimization**: Debounced syntax highlighting and spell checking via timers.

---

## Core Features & Improvements

### Live Preview Architecture (Active Block Swapping)
- [x] **Migrate to QTextEdit**: Change `EditorWidget` base class from `QPlainTextEdit` to `QTextEdit` to support rich text layouts, tables, and inline images.
- [ ] **Active Block Tracking**: Implement logic in `EditorWidget` to track cursor movement and identify the currently active `QTextBlock` (the line being edited).
- [ ] **Block Swapping Logic**: 
    - When a block is active, display raw Markdown notation for editing.
    - When a block becomes inactive, replace its content with fully rendered Rich Text (hiding syntax characters like `**` or `#`).
- [ ] **Markdown State Management**: Store the original raw Markdown string in `QTextBlockUserData` so it can be perfectly restored when the user clicks back into the rendered text block.

### Rich Text Features (Requires QTextEdit Migration)
- [ ] **Inline Image Support**: Display actual images within the editor view when an image block is inactive.
- [ ] **Advanced Table Support**:
    - Render actual rich-text tables when a block containing Markdown tables is inactive.
    - Handle transitions between a rendered table and the raw text representation gracefully.
- [ ] **True Horizontal Rules**: Render actual visual lines instead of styled `---` text.

### UI/UX Enhancements
- [ ] **Find & Replace**: Add replace functionality to `FindBarWidget`.
- [ ] **Recent Files List**: Track last 10-20 opened files in `QSettings`.
- [ ] **Session Management**: Save open files, window size/position on exit and restore on launch.
- [ ] **Font Customization**: Add font family selector in preferences.
- [ ] **Customizable Keyboard Shortcuts**: Move hardcoded shortcuts to configurable key bindings.
- [ ] **Customizable Themes**: Allow users to create/edit custom themes and import/export JSON.

### Theme System Improvements
- [ ] **Secondary Color Support**: Add secondary/accent color field to theme definitions.
- [ ] **Theme Completeness**: Ensure all UI elements use theme colors (no hardcoded values).

### Code Quality & Maintainability
- [ ] **Add Unit Tests**: Set up Qt Test framework.
- [ ] **Code Documentation**: Add Doxygen-style comments to public APIs.

### Packaging & Distribution
- [ ] **Linux Packaging**: Create `.deb`, `.rpm`, and AppImage.
- [ ] **Windows Support**: Create Windows installer.
- [ ] **macOS Support**: Create `.app` bundle.
- [ ] **Continuous Integration**: Set up GitHub Actions for automated builds.

---

## Under Consideration (Not Priority)

- [ ] **Mermaid Graph Rendering**: Render Mermaid diagrams inline (Requires JavaScript engine or external rendering; high complexity).
- [ ] **Markdown Preview Pane**: Explicitly not part of core design philosophy (Live Preview replaces this need entirely).

---

## Known Issues

- [x] **Underscore Handling in Technical Text**: Variables like `my_variable_name` incorrectly trigger italic formatting.
- [x] **Sidebar Path Not Syncing**: When opening files, sidebar doesn't navigate to file's directory.
- [x] **Hardcoded Accent Colors**: Blue color hardcoded in multiple places instead of using theme colors.