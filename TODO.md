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

### Markdown Rendering & Parsing
- [ ] **Improve Markdown Parsing Robustness**:
    - Fix edge cases with underscores in technical text (e.g., `variable_name_test` should not trigger italic)
    - Better handling of inline code with special characters
    - Ensure proper escaping and boundary detection for all Markdown elements
    - Review and refine regex patterns in `MarkdownHighlighter::setupInitialRules()`

- [ ] **Advanced Table Support**:
    - Add visual borders for tables (currently only background coloring)
    - Improve table cell alignment options
    - Handle multi-line table cells
    - Add table editing helpers (insert row/column via context menu)

- [ ] **True Horizontal Rules**:
    - Render actual visual lines instead of styled `---` text
    - Support all Markdown horizontal rule syntax (`---`, `***`, `___`)

- [ ] **Improved Image Handling**:
    - Display image placeholders with dimensions
    - Show alt text with better visual treatment
    - Consider optional inline image rendering (with performance safeguards)

### UI/UX Enhancements

- [ ] **Text Justification**:
    - Add full text justification for long lines
    - Ensure proper hyphenation handling
    - Make justification optional (configurable per user preference)

- [ ] **Find & Replace**:
    - Add replace functionality to `FindBarWidget`
    - Support replace single / replace all operations
    - Add regex replace option

- [ ] **Find Result Highlighting**:
    - Highlight all occurrences of search term in document
    - Show match count in find bar
    - Navigate between matches with keyboard shortcuts

- [ ] **Recent Files List**:
    - Track last 10-20 opened files in `QSettings`
    - Add "Open Recent" submenu in File menu
    - Clear recent files option

- [ ] **Session Management**:
    - Save open files, window size/position on exit
    - Restore session on application launch
    - Optional: named sessions support

- [ ] **Font Customization**:
    - Add font family selector in preferences
    - Allow base font size adjustment (beyond zoom)
    - Persist font preferences in settings

- [ ] **Customizable Keyboard Shortcuts**:
    - Move hardcoded shortcuts to configurable key bindings
    - Add preferences dialog for shortcut customization
    - Support conflict detection

- [ ] **Customizable Themes**:
    - Allow users to create/edit custom themes
    - Support theme import/export (JSON format)
    - Add theme preview in preferences

### Theme System Improvements

- [ ] **Secondary Color Support**:
    - Add secondary/accent color field to theme definitions
    - Replace hardcoded blue (`#0366d6` / `#58a6ff`) with theme secondary color
    - Apply secondary color to: links, focus highlights, selection, buttons
    - Migrate existing themes with appropriate secondary colors:
        - Light: `#0366d6` (current blue)
        - Dark: `#58a6ff` (current blue)
        - Pitch Black: `#64b4ff` (slightly lighter blue for contrast)

- [ ] **Theme Completeness**:
    - Ensure all UI elements use theme colors (no hardcoded values)
    - Add syntax highlighting color customization
    - Support for additional UI component styling

### Sidebar Improvements

- [ ] **Fix Sidebar Path on File Open**:
    - When opening a file (especially via command line), set sidebar to file's directory
    - Currently shows default root instead of file location

- [ ] **Editable Path in Sidebar**:
    - Make path field in `SidebarFileExplorer` editable on click/focus
    - Allow manual path entry with validation
    - Refresh directory on Enter key or refresh button

### Code Quality & Maintainability

- [ ] **Add Unit Tests**:
    - Set up Qt Test framework
    - Test `FileManager` operations (load, save, export)
    - Test `MarkdownHighlighter` rules
    - Test `FindBarWidget` functionality
    - Add integration tests for main workflows

- [ ] **Code Documentation**:
    - Add Doxygen-style comments to public APIs
    - Document class relationships and architecture
    - Add inline comments for complex logic

- [ ] **Refactor Complex Functions**:
    - Break down `MarkdownHighlighter::highlightBlock()` (200+ lines)
    - Simplify `MarkdownHighlighter::updateFormatsForTheme()`
    - Improve `EditorWidget::contextMenuEvent()` readability

### Packaging & Distribution

- [ ] **Linux Packaging**:
    - Create `.deb` package for Debian/Ubuntu
    - Create `.rpm` package for Fedora/openSUSE
    - Create AppImage for universal Linux distribution
    - Consider Flatpak packaging

- [ ] **Windows Support**:
    - Create Windows installer (NSIS or Inno Setup)
    - Fix any platform-specific issues
    - Bundle required DLLs and dependencies

- [ ] **macOS Support**:
    - Create `.app` bundle
    - Code signing for macOS distribution
    - Notarization for Gatekeeper

- [ ] **Continuous Integration**:
    - Set up GitHub Actions for automated builds
    - Run tests on push/PR
    - Generate release artifacts automatically
    - Support Windows, macOS, and Linux build matrices

---

## Under Consideration (Not Priority)

*These features are complex and currently under review. Implementation is not guaranteed.*

- [ ] **Inline Images**:
    - **Concept**: Display actual images within the editor view
    - **Hurdles**: `QPlainTextEdit` limitations; may require custom rendering or `QTextEdit` (performance trade-off)

- [ ] **Mermaid Graph Rendering**:
    - **Concept**: Render Mermaid diagrams inline
    - **Hurdles**: Requires JavaScript engine or external rendering; high complexity

- [ ] **Markdown Preview Pane**:
    - **Concept**: Optional side panel showing rendered HTML
    - **Status**: Explicitly not part of core design philosophy
    - **Decision**: Keep as "won't implement" unless user demand increases

---

## Known Issues

- [ ] **Underscore Handling in Technical Text**: Variables like `my_variable_name` incorrectly trigger italic formatting
- [ ] **Sidebar Path Not Syncing**: When opening files, sidebar doesn't navigate to file's directory
- [ ] **Hardcoded Accent Colors**: Blue color hardcoded in multiple places instead of using theme colors
- [ ] **No Text Justification**: Lines are left-aligned only, affecting readability on wide screens
