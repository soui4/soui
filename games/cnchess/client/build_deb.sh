#!/bin/bash

# cnchess (Happy Chinese Chess) Debian Package Builder
# Derived from the xmusic build_deb.sh script.
#
# Key difference vs xmusic:
#   xmusic links everything statically and ships a single executable.
#   cnchess is usually built in DLL mode (SOUI_ENABLE_CORE_LIB/SOUI_ENABLE_COM_LIB off),
#   so besides cnchess_client we must collect every SOUI4 module shared library.
#   The Windows reference layout lives in D:\avatar\Release and contains:
#     cnchess_client.exe, soui4.dll, utilities4.dll, render-skia.dll,
#     imgdecoder-stb.dll, resprovider-zip.dll, log4z.dll, taskloop.dll,
#     ws.dll, soui-sys-resource.dll, cnchess_cfg.xml
#   On Linux those become lib<name>.so files next to the executable.
#
# This script handles both build flavors automatically:
#   - DLL mode   (bin/libsoui4.so exists): exe + all module .so files
#                are installed into /usr/local/lib/happychess, and a launcher
#                script /usr/local/bin/happychess sets LD_LIBRARY_PATH.
#   - Static mode (no libsoui4.so): behaves like xmusic, only the exe
#                is packaged (launcher still works, LD_LIBRARY_PATH is a no-op).
#
set -e

echo "==================================="
echo "happychess Debian Package Builder"
echo "==================================="
echo ""

# Parse command line arguments
TARGET_DISTRO="generic"  # Default to generic Linux
BUILD_DIR_OVERRIDE=""

while [[ $# -gt 0 ]]; do
    case $1 in
        --target=*)
            TARGET_DISTRO="${1#*=}"
            shift
            ;;
        --target)
            TARGET_DISTRO="$2"
            shift 2
            ;;
        --build-dir=*)
            BUILD_DIR_OVERRIDE="${1#*=}"
            shift
            ;;
        --build-dir)
            BUILD_DIR_OVERRIDE="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --target=DISTRO     Target distribution (deepin, ubuntu, generic)"
            echo "  --build-dir=DIR     CMake build directory containing bin/cnchess_client"
            echo "                      (default: auto-detect <soui4-root>/build_rel or build)"
            echo "  -h, --help          Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0                                    # auto-detect build dir"
            echo "  $0 --build-dir=/path/to/soui4/build_rel"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Configuration
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
# script lives at <soui4-root>/games/cnchess/client -> three levels up is the repo root
SOUI_ROOT="$(cd "$PROJECT_DIR/../../.." && pwd)"
PACKAGE_DIR="$PROJECT_DIR/debian_package"
OUTPUT_DEB="$PROJECT_DIR/happychess.deb"

# Icon source (rendered to png by the svgcvt tool from the build output)
ICON_SOURCE_SVG="$PROJECT_DIR/uires/svg/icon.svg"
# Icon sizes to generate (following freedesktop.org standards)
ICON_SIZES=(16 32 48 64 128 256)

# Package metadata
PKG_NAME="happychess"
PKG_VERSION="1.0.0"
APP_HOME="/usr/local/lib/happychess"   # exe + modules + default config live here
EXE_NAME="cnchess_client"

# Default game server written into the shipped cnchess_cfg.xml.
# The app reads/writes this file in its application directory (GetAppDir()).
DEFAULT_SERVER="124.222.50.176:3311"

# Module shared libraries to collect in DLL mode.
# Mapping from the Windows Release folder (D:\avatar\Release):
#   soui4.dll            -> libsoui4.so           (DT_NEEDED, linked into exe)
#   utilities4.dll       -> libutilities4.so      (DT_NEEDED)
#   (static on Windows)  -> libswinx.so           (DT_NEEDED, Linux platform layer)
#   render-skia.dll      -> librender-skia.so     (dlopen "librender-skia" + ".so" by SComLoader)
#   imgdecoder-stb.dll   -> libimgdecoder-stb.so  (dlopen "libimgdecoder-stb")
#   log4z.dll            -> liblog4z.so           (dlopen "liblog4z", SAppCfg::SetLog)
#   taskloop.dll         -> libtaskloop.so        (dlopen "libtaskloop", SComMgr2/ThemeDownloadManager)
#   ws.dll               -> libws.so              (dlopen "libws", SComMgr2::CreateWS/WebSocketClient)
#   resprovider-zip.dll  -> libresprovider-zip.so (DT_NEEDED, linked into exe)
# ExtendCtrls and cnchess_algorithm are always STATIC libraries and are
# already linked into the executable; they must NOT be collected.
REQUIRED_MODULES=(
    libsoui4.so
    libswinx.so
    libcairo.so
    libutilities4.so
    libresprovider-zip.so
    librender-skia.so
    libimgdecoder-stb.so
    liblog4z.so
    libtaskloop.so
    libws.so
    libmsftedit.so
)

# Display target distribution
if [ "$TARGET_DISTRO" != "generic" ]; then
    echo "Target Distribution: $TARGET_DISTRO"
else
    echo "Target Distribution: Generic Linux"
fi
echo ""

# ---------------------------------------------------------------
# Locate the build directory containing bin/cnchess_client
# ---------------------------------------------------------------
if [ -n "$BUILD_DIR_OVERRIDE" ]; then
    BUILD_DIR="$BUILD_DIR_OVERRIDE"
else
    BUILD_DIR=""
    for cand in \
        "$SOUI_ROOT/build_rel" \
        "$SOUI_ROOT/build" \
        "$SOUI_ROOT/out/build_rel" \
        "$SOUI_ROOT/out/build" \
        "$PROJECT_DIR/build_rel" \
        "$PROJECT_DIR/build"
    do
        if [ -f "$cand/bin/$EXE_NAME" ] || [ -f "$cand/bin/Release/$EXE_NAME" ]; then
            BUILD_DIR="$cand"
            break
        fi
    done
fi

if [ -z "$BUILD_DIR" ]; then
    echo "Error: could not locate a build directory containing bin/$EXE_NAME"
    echo "Searched:"
    echo "  $SOUI_ROOT/build_rel"
    echo "  $SOUI_ROOT/build"
    echo "  $SOUI_ROOT/out/build_rel"
    echo "  $SOUI_ROOT/out/build"
    echo "  $PROJECT_DIR/build_rel"
    echo "  $PROJECT_DIR/build"
    echo ""
    echo "Build the project first, e.g.:"
    echo "  cd $SOUI_ROOT"
    echo "  cmake -S . -B build_rel -DCMAKE_BUILD_TYPE=Release"
    echo "  cmake --build build_rel --parallel"
    echo ""
    echo "Or pass the build dir explicitly: $0 --build-dir=/path/to/build_rel"
    exit 1
fi

# Normalize to the directory that actually holds the exe
if [ -f "$BUILD_DIR/bin/$EXE_NAME" ]; then
    BIN_DIR="$BUILD_DIR/bin"
else
    BIN_DIR="$BUILD_DIR/bin/Release"
fi

if [ ! -f "$BIN_DIR/$EXE_NAME" ]; then
    echo "Error: $EXE_NAME not found at $BIN_DIR/$EXE_NAME"
    exit 1
fi
echo "Build directory : $BUILD_DIR"
echo "Binary directory: $BIN_DIR"

# svgcvt lives next to cnchess_client in the build output (built when
# SOUI_ENABLE_SVG=ON, which is the default). Assigned here on purpose:
# BIN_DIR must be resolved first.
SVG2PNG_TOOL="$BIN_DIR/svgcvt"

# ---------------------------------------------------------------
# Detect build flavor (DLL vs static) and collect the module list
# ---------------------------------------------------------------
MODE="static"
MODULES=()
if [ -f "$BIN_DIR/libsoui4.so" ]; then
    MODE="dll"
    echo ""
    echo "Detected DLL build (libsoui4.so found). Collecting SOUI4 modules..."
    for mod in "${REQUIRED_MODULES[@]}"; do
        if [ ! -f "$BIN_DIR/$mod" ]; then
            echo "Error: required module not found: $BIN_DIR/$mod"
            echo "The DLL-mode build should have produced it next to the executable."
            exit 1
        fi
        MODULES+=("$mod")
        echo "  + $mod"
    done
else
    echo ""
    echo "Detected static build (no libsoui4.so). Packaging a single executable,"
    echo "just like xmusic."
fi
echo ""

# ---------------------------------------------------------------
# Clean previous package directory and create the structure
# ---------------------------------------------------------------
echo "Cleaning previous package directory..."
rm -rf "$PACKAGE_DIR"

echo "Creating package structure..."
mkdir -p "$PACKAGE_DIR/DEBIAN"
mkdir -p "$PACKAGE_DIR$APP_HOME"
mkdir -p "$PACKAGE_DIR/usr/local/bin"
mkdir -p "$PACKAGE_DIR/usr/local/share/applications"
for size in "${ICON_SIZES[@]}"; do
    mkdir -p "$PACKAGE_DIR/usr/local/share/icons/hicolor/${size}x${size}/apps"
done

# ---------------------------------------------------------------
# Copy executable and modules
# ---------------------------------------------------------------
echo "Copying executable..."
cp "$BIN_DIR/$EXE_NAME" "$PACKAGE_DIR$APP_HOME/$EXE_NAME"
chmod 755 "$PACKAGE_DIR$APP_HOME/$EXE_NAME"

if [ "$MODE" = "dll" ]; then
    echo "Copying SOUI4 module libraries..."
    for mod in "${MODULES[@]}"; do
        cp "$BIN_DIR/$mod" "$PACKAGE_DIR$APP_HOME/$mod"
        chmod 755 "$PACKAGE_DIR$APP_HOME/$mod"
    done

    # Patch RPATH so the exe and libraries also work when launched directly
    # (without the launcher script). $ORIGIN resolves to $APP_HOME itself.
    if command -v patchelf >/dev/null 2>&1; then
        echo "Patching RPATH (\$ORIGIN) with patchelf..."
        patchelf --set-rpath '$ORIGIN' "$PACKAGE_DIR$APP_HOME/$EXE_NAME"
        for mod in "${MODULES[@]}"; do
            patchelf --set-rpath '$ORIGIN' "$PACKAGE_DIR$APP_HOME/$mod"
        done
    else
        echo "Note: patchelf not found, skipping RPATH patch."
        echo "      The launcher script still works via LD_LIBRARY_PATH."
    fi
fi

# ---------------------------------------------------------------
# Default configuration file
# ---------------------------------------------------------------
# The app reads/writes cnchess_cfg.xml in its application directory
# (SApplication::GetAppDir(), i.e. the directory of the executable).
# Mode 666 so regular users can persist their profile; on Windows this
# file simply sits next to cnchess_client.exe in the Release folder.
# NOTE: the app also creates log/ and theme_cache/ in the same directory;
# postinst pre-creates them with permissive modes (see below).
echo "Creating default configuration file..."
cat > "$PACKAGE_DIR$APP_HOME/cnchess_cfg.xml" << EOF
<?xml version="1.0"?><config svr="$DEFAULT_SERVER" name="" sex="0" avatar_id="1" avatar_path=""/>
EOF
chmod 666 "$PACKAGE_DIR$APP_HOME/cnchess_cfg.xml"

# ---------------------------------------------------------------
# Launcher script /usr/local/bin/happychess
# ---------------------------------------------------------------
echo "Creating launcher script..."
cat > "$PACKAGE_DIR/usr/local/bin/happychess" << EOF
#!/bin/bash
# Launcher for $EXE_NAME (SOUI4 Chinese chess client)
APP_HOME="$APP_HOME"
export LD_LIBRARY_PATH="\$APP_HOME\${LD_LIBRARY_PATH:+:\$LD_LIBRARY_PATH}"
exec "\$APP_HOME/$EXE_NAME" "\$@"
EOF
chmod 755 "$PACKAGE_DIR/usr/local/bin/happychess"

# Generate application icons from SVG using the svgcvt tool
echo "Generating application icons from SVG..."
if [ -f "$ICON_SOURCE_SVG" ]; then
    if [ -x "$SVG2PNG_TOOL" ]; then
        echo "Using svgcvt tool to generate icons..."
        
        # Generate icons for all required sizes
        for size in "${ICON_SIZES[@]}"; do
            icon_dir="$PACKAGE_DIR/usr/local/share/icons/hicolor/${size}x${size}/apps"
            icon_file="$icon_dir/$PKG_NAME.png"
            
            echo "  Generating ${size}x${size} icon..."
            "$SVG2PNG_TOOL" -i "$ICON_SOURCE_SVG" -o "$icon_file" -s "$size"
            
            if [ -f "$icon_file" ]; then
                echo "    ✓ ${size}x${size} icon generated successfully"
            else
                echo "    ✗ Failed to generate ${size}x${size} icon"
            fi
        done
        
        echo "✓ All icons generated successfully using svgcvt"
    else
        echo "⚠ Warning: svgcvt tool not found at $SVG2PNG_TOOL"
        echo "  Please build the project first to generate the svgcvt tool"
        echo "  (requires SOUI_ENABLE_SVG=ON, which is the default)."
        exit 1
    fi
else
    echo "⚠ Warning: SVG icon file not found at $ICON_SOURCE_SVG"
    echo "  The package will be built without an icon."
fi

# ---------------------------------------------------------------
# DEBIAN/control
# ---------------------------------------------------------------
# Runtime dependencies of a DLL-mode cnchess build on Linux:
#   - swinx (Linux platform layer) links: dl, xcb, uuid, atomic, libstdc++
#     and optionally ALSA (HAS_ALSA) for sound effects
#   - window/input via XCB (bundled), OpenGL via libgl1 for render-skia
#   - CJK fonts are recommended for the Chinese UI
case "$TARGET_DISTRO" in
    deepin)
        DEPENDS="libc6 (>= 2.28), libstdc++6, libgcc-s1 | libgcc1, libx11-6, libxcb1, libgl1, libasound2, libuuid1"
        echo "Using Deepin-specific dependencies"
        ;;
    ubuntu)
        DEPENDS="libc6 (>= 2.31), libstdc++6, libgcc-s1 | libgcc1, libx11-6, libxcb1, libgl1, libasound2, libuuid1"
        echo "Using Ubuntu-optimized dependencies"
        ;;
    *)
        DEPENDS="libc6 (>= 2.17), libstdc++6, libgcc-s1 | libgcc1, libx11-6, libxcb1, libgl1, libasound2, libuuid1"
        echo "Using generic Linux dependencies"
        ;;
esac

# Detect package architecture
if command -v dpkg >/dev/null 2>&1; then
    ARCH="$(dpkg --print-architecture)"
else
    case "$(uname -m)" in
        x86_64)          ARCH="amd64" ;;
        aarch64|arm64)   ARCH="arm64" ;;
        *)               ARCH="$(uname -m)" ;;
    esac
fi

echo "Creating control file (architecture: $ARCH)..."
cat > "$PACKAGE_DIR/DEBIAN/control" << EOF
Package: $PKG_NAME
Version: $PKG_VERSION
Section: games
Priority: optional
Architecture: $ARCH
Depends: $DEPENDS
Recommends: fonts-noto-cjk | fonts-wqy-microhei | fonts-wqy-zenhei
Maintainer: happychess Team <setoutsoft@qq.com>
Homepage: https://gitee.com/setoutsoft/soui
Description: Online Chinese chess (xiangqi) client based on SOUI4 framework
 happychess is an online Chinese chess game client built with the SOUI4
 Direct UI framework, connecting to a dedicated game server for
 multiplayer matches.
 .
 Key Features:
 - Online lobby with real-time table list and seat management
 - Human vs human, human vs robot (built-in PVS search engine) games
 - Classic endgame puzzles with automatic validation
 - In-game chat and peace (draw) negotiation
 - Downloadable themes with automatic caching
 .
 Technical Highlights:
 - Built with SOUI4 Direct UI framework for declarative XML layouts
 - WebSocket-based real-time protocol (libws component)
 - Chess AI engine with transposition table and killer moves
 - Resources embedded into the executable for single-file deployment
EOF

# ---------------------------------------------------------------
# DEBIAN/postinst
# ---------------------------------------------------------------
echo "Creating post-installation script..."
cat > "$PACKAGE_DIR/DEBIAN/postinst" << 'EOF'
#!/bin/bash
set -e

# The app writes cnchess_cfg.xml, log/ and theme_cache/ next to its
# executable (SApplication::GetAppDir()). Pre-create those with
# permissive modes so regular (non-root) users can run the game.
APP_HOME="/usr/local/lib/happychess"
if [ -d "$APP_HOME" ]; then
    if [ -f "$APP_HOME/cnchess_cfg.xml" ]; then
        chmod 666 "$APP_HOME/cnchess_cfg.xml" || true
    fi
    mkdir -p "$APP_HOME/log" "$APP_HOME/theme_cache" 2>/dev/null || true
    chmod 777 "$APP_HOME/log" "$APP_HOME/theme_cache" 2>/dev/null || true
fi

# Update desktop database if available
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database /usr/local/share/applications || true
fi

# Update icon cache if available
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f /usr/local/share/icons/hicolor || true
fi

echo "happychess has been successfully installed!"
echo "You can launch it from the applications menu or by running 'happychess' in terminal."

exit 0
EOF
chmod 755 "$PACKAGE_DIR/DEBIAN/postinst"

# ---------------------------------------------------------------
# DEBIAN/prerm
# ---------------------------------------------------------------
echo "Creating pre-removal script..."
cat > "$PACKAGE_DIR/DEBIAN/prerm" << 'EOF'
#!/bin/bash
set -e

# Clean up desktop database if available
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database /usr/local/share/applications || true
fi

# Clean up icon cache if available
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f /usr/local/share/icons/hicolor || true
fi

exit 0
EOF
chmod 755 "$PACKAGE_DIR/DEBIAN/prerm"

# ---------------------------------------------------------------
# Desktop entry
# ---------------------------------------------------------------
echo "Creating desktop entry..."
cat > "$PACKAGE_DIR/usr/local/share/applications/$PKG_NAME.desktop" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=Happy Chinese Chess
Name[zh_CN]=开心象棋
GenericName=Chinese Chess Game
GenericName[zh_CN]=中国象棋
Comment=Online Chinese chess (xiangqi) client with multiplayer and robot games
Comment[zh_CN]=开心象棋客户端：联机对弈、人机对弈、残局闯关
Exec=happychess
Icon=$PKG_NAME
Terminal=false
Categories=Game;BoardGame;
Keywords=chess;xiangqi;happychess;象棋;棋类;
StartupNotify=true
StartupWMClass=happychess
EOF
chmod 644 "$PACKAGE_DIR/usr/local/share/applications/$PKG_NAME.desktop"

# ---------------------------------------------------------------
# Build the package
# ---------------------------------------------------------------
echo ""
echo "Building Debian package..."

# Verify all required files exist
echo "Verifying package structure..."
REQUIRED_FILES=(
    "$PACKAGE_DIR/DEBIAN/control"
    "$PACKAGE_DIR/DEBIAN/postinst"
    "$PACKAGE_DIR/DEBIAN/prerm"
    "$PACKAGE_DIR$APP_HOME/$EXE_NAME"
    "$PACKAGE_DIR/usr/local/bin/happychess"
    "$PACKAGE_DIR/usr/local/share/applications/$PKG_NAME.desktop"
)

for file in "${REQUIRED_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo "Error: Required file not found: $file"
        exit 1
    fi
    echo "  OK $file"
done

if [ "$MODE" = "dll" ]; then
    for mod in "${MODULES[@]}"; do
        if [ ! -f "$PACKAGE_DIR$APP_HOME/$mod" ]; then
            echo "Error: Required module not found in package: $mod"
            exit 1
        fi
        echo "  OK $APP_HOME/$mod"
    done
fi

# Generate md5sums file (required for proper DEB packages)
echo "Generating md5sums..."
cd "$PACKAGE_DIR"
find . -type f ! -path './DEBIAN/*' -exec md5sum {} \; | sed 's| \./| |' > DEBIAN/md5sums
chmod 644 DEBIAN/md5sums
echo "OK md5sums file generated"

# List DEBIAN directory contents for debugging
echo ""
echo "DEBIAN directory contents:"
ls -la "$PACKAGE_DIR/DEBIAN/"
echo ""

echo "Installed files:"
find "$PACKAGE_DIR/usr" -type f | while read -r f; do
    echo "  ${f#$PACKAGE_DIR}"
done
echo ""

cd "$PROJECT_DIR"

# Pick a dpkg-deb invocation that yields sane ownership:
# root -> as-is; otherwise prefer fakeroot (file owner in the package
# would otherwise be the building user).
if [ "$(id -u)" = "0" ]; then
    DPKG_BUILD=(dpkg-deb -Zgzip --uniform-compression --build)
elif command -v fakeroot >/dev/null 2>&1; then
    DPKG_BUILD=(fakeroot dpkg-deb -Zgzip --uniform-compression --build)
else
    echo "Warning: not running as root and fakeroot not found;"
    echo "  package file ownership will be the current user."
    DPKG_BUILD=(dpkg-deb -Zgzip --uniform-compression --build)
fi

echo "Running dpkg-deb --build..."
"${DPKG_BUILD[@]}" "$PACKAGE_DIR" "$OUTPUT_DEB"

# Verify the package
echo ""
echo "Verifying package..."
dpkg-deb --info "$OUTPUT_DEB"

echo ""
echo "==================================="
echo "Package built successfully!"
echo "Output: $OUTPUT_DEB"
echo "==================================="
echo ""
echo "To install the package, run:"
echo "  sudo dpkg -i $OUTPUT_DEB"
echo ""
echo "To remove the package, run:"
echo "  sudo dpkg -r $PKG_NAME"
echo ""
