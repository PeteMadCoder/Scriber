#!/bin/bash
# Scriber Markdown Editor Installation Script
# This script installs the Scriber Markdown editor on Linux systems

set -e  # Exit on error

# Detect if running as root
IS_ROOT=0
if [ "$(id -u)" = "0" ]; then
    IS_ROOT=1
fi

# Configuration
APP_NAME="Scriber"
EXECUTABLE_NAME="scriber"
REPO_DIR="$PWD"
INSTALL_PREFIX=""
DESKTOP_FILE="scriber.desktop"
ICON_NAME="appicon.png"
ICON_SIZE="256x256"
BUILD_DIR="${REPO_DIR}/build"
RESOURCES_DIR="${REPO_DIR}/resources"
THEME_DIR="${RESOURCES_DIR}/themes"
ICON_DIR="${RESOURCES_DIR}/icons"

# Determine installation prefix
if [ $IS_ROOT -eq 1 ]; then
    INSTALL_PREFIX="/usr"
    echo "Installing system-wide (requires root privileges)"
else
    INSTALL_PREFIX="$HOME/.local"
    echo "Installing for current user only"
fi

# Check for required dependencies
check_dependencies() {
    echo "Checking for required dependencies..."
    
    # Check for CMake
    if ! command -v cmake &> /dev/null; then
        echo "Error: cmake is not installed. Please install it first."
        echo "On Debian/Ubuntu: sudo apt install cmake"
        echo "On Fedora: sudo dnf install cmake"
        echo "On Arch: sudo pacman -S cmake"
        exit 1
    fi
    
    # Check for Qt 6
    if ! command -v qmake6 &> /dev/null && ! command -v qmake &> /dev/null; then
        echo "Error: Qt 6 is not installed. Please install it first."
        echo "On Debian/Ubuntu: sudo apt install qt6-base-dev qt6-tools-dev qt6-tools-dev-tools"
        echo "On Fedora: sudo dnf install qt6-qtbase-devel qt6-qttools-devel"
        echo "On Arch: sudo pacman -S qt6-base qt6-tools"
        exit 1
    fi
    
    # Check for build essentials
    if ! command -v g++ &> /dev/null || ! command -v make &> /dev/null; then
        echo "Error: Build essentials are not installed. Please install them first."
        echo "On Debian/Ubuntu: sudo apt install build-essential"
        echo "On Fedora: sudo dnf groupinstall 'Development Tools'"
        echo "On Arch: sudo pacman -S base-devel"
        exit 1
    fi
}

# Build the application
build_application() {
    echo "Building ${APP_NAME}..."
    
    # Create build directory if it doesn't exist
    if [ ! -d "${BUILD_DIR}" ]; then
        mkdir -p "${BUILD_DIR}"
    fi
    
    # Configure with CMake
    cd "${BUILD_DIR}"
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" ..
    
    # Build
    cmake --build . --config Release
    
    cd "${REPO_DIR}"
}

# Install the application
install_application() {
    echo "Installing ${APP_NAME} to ${INSTALL_PREFIX}..."
    
    # Install executable
    EXECUTABLE_SRC="${BUILD_DIR}/${EXECUTABLE_NAME}"
    if [ ! -f "${EXECUTABLE_SRC}" ]; then
        echo "Error: Executable not found at ${EXECUTABLE_SRC}. Did you build the application?"
        exit 1
    fi
    
    BIN_DIR="${INSTALL_PREFIX}/bin"
    mkdir -p "${BIN_DIR}"
    cp "${EXECUTABLE_SRC}" "${BIN_DIR}/${EXECUTABLE_NAME}"
    chmod +x "${BIN_DIR}/${EXECUTABLE_NAME}"
    
    # Install resources
    if [ -d "${THEME_DIR}" ]; then
        RESOURCE_DIR="${INSTALL_PREFIX}/share/${EXECUTABLE_NAME}/themes"
        mkdir -p "${RESOURCE_DIR}"
        cp -r "${THEME_DIR}"/* "${RESOURCE_DIR}/"
    fi
    
    # Install icons
    if [ -d "${ICON_DIR}" ] && [ -f "${ICON_DIR}/${ICON_NAME}" ]; then
        ICON_INSTALL_DIR="${INSTALL_PREFIX}/share/icons/hicolor/${ICON_SIZE}/apps"
        mkdir -p "${ICON_INSTALL_DIR}"
        cp "${ICON_DIR}/${ICON_NAME}" "${ICON_INSTALL_DIR}/${EXECUTABLE_NAME}.png"
        
        # Update icon cache
        if [ $IS_ROOT -eq 1 ]; then
            gtk-update-icon-cache -f -t /usr/share/icons/hicolor
        else
            gtk-update-icon-cache -f -t "${HOME}/.local/share/icons/hicolor"
        fi
    fi
    
    # Install desktop file
    DESKTOP_FILE_PATH="${INSTALL_PREFIX}/share/applications/${DESKTOP_FILE}"
    mkdir -p "$(dirname "${DESKTOP_FILE_PATH}")"
    
    cat > "${DESKTOP_FILE_PATH}" << EOF
[Desktop Entry]
Name=${APP_NAME}
Comment=Distraction-free Markdown Editor
Exec=${EXECUTABLE_NAME} %F
Icon=${EXECUTABLE_NAME}
Terminal=false
Type=Application
Categories=TextEditor;Utility;
MimeType=text/markdown;text/x-markdown;
StartupNotify=true
EOF

    # Update desktop database
    if [ $IS_ROOT -eq 1 ]; then
        update-desktop-database /usr/share/applications
    else
        update-desktop-database "${HOME}/.local/share/applications"
    fi
    
    # Register MIME type for Markdown files
    if [ -x "$(command -v xdg-mime)" ]; then
        xdg-mime default "${DESKTOP_FILE}" text/markdown text/x-markdown
    fi
    
    echo -e "\n${APP_NAME} has been successfully installed!"
    echo "You can launch it by typing '${EXECUTABLE_NAME}' in the terminal"
    echo "or find it in your application menu under 'Text Editors'"
}

# Main installation process
main() {
    echo "=========================================="
    echo " Installing ${APP_NAME}"
    echo "=========================================="
    
    check_dependencies
    
    # Build always to ensure latest changes are included
    build_application
    
    install_application
    
    echo -e "\nInstallation complete!"
}

# Run the main function
main "$@"