#!/bin/bash

# fundholdings (Fund Holdings Assistant) Debian Package Builder
# Derived from games/cnchess/client/build_deb.sh
#
# Key differences vs the cnchess client:
#   1. cnchess embeds all UI resources into the executable and ships a single
#      binary. On Linux this demo loads its layouts/skins from
#        <appdir>/uires                  (add_app_res_folder)
#      System resources are NEVER loaded from a zip package on any platform:
#      every skin, font and scrollbar skin used by this app is defined inside
#      uires. On Apple the dark system theme (soui-sys-resource-dark) is copied
#      into the .app bundle and loaded from there with SetSysResFile().
#   2. The demo explicitly links the httpclient COM component (IHttpClient is
#      used to fetch fund quotes/holdings). On Linux that component is built on
#      top of a statically linked libcurl, so HTTPS needs system CA certs.
#   3. The watch list is read/written as <appdir>/fundholdings_watch.txt.
#      It is shipped as a dpkg conffile (upgrades keep the user's edits) and
#      made world-writable by postinst so non-root users can save their funds.
#
# Only the runtime modules declared by CMake in fundholdings-runtime.list are
# packaged (with a built-in fallback list if the manifest is missing). Optional
# components that the root CMake builds for other demos stay out of the .deb:
#   libws.so, libnetwork.so, librender-gdi.so, libresprovider-7zip.so,
#   libresprovider-zip.so, libscriptmodule-lua.so, libsipcobject.so
# No system-resource zip is shipped on any platform.
#
# Both build flavors are handled automatically:
#   - DLL mode   (bin/libsoui4.so exists): exe + whitelisted module .so files
#                are installed into /usr/local/lib/fundholdings, and
#                /usr/local/bin/fundholdings sets LD_LIBRARY_PATH.
#   - Static mode (no libsoui4.so): only the exe plus whitelisted component
#                .so files that still exist are packaged.
#
set -e

echo "====================================="
echo "fundholdings Debian Package Builder"
echo "====================================="
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
            echo "  --build-dir=DIR     CMake build directory containing bin/fundholdings"
            echo "                      (or the bin directory itself)"
            echo "                      (default: auto-detect <soui4-root>/build_rel or build)"
            echo "  -h, --help          Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0                                    # auto-detect build dir"
            echo "  $0 --build-dir=/path/to/soui4/build_rel"
            echo "  $0 --target=ubuntu --build-dir=/path/to/soui4/build_rel/bin/Release"
            echo ""
            echo "Build the project first, e.g.:"
            echo "  cmake -S <soui4-root> -B build_rel -DCMAKE_BUILD_TYPE=Release"
            echo "  cmake --build build_rel --parallel"
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
# script lives at <soui4-root>/demos/fundholdings -> two levels up is the repo root
SOUI_ROOT="$(cd "$PROJECT_DIR/../.." && pwd)"
PACKAGE_DIR="$PROJECT_DIR/debian_package"
OUTPUT_DEB="$PROJECT_DIR/fundholdings.deb"

# This script is meant to run on the Linux build machine (Debian/Ubuntu).
# On Windows (Git Bash / MSYS) there is no dpkg-deb: fail early with a clear
# message instead of failing halfway through the package layout.
if ! command -v dpkg-deb >/dev/null 2>&1; then
    echo "Error: dpkg-deb not found. Run this script on a Debian/Ubuntu host"
    echo "       (apt install dpkg fakeroot patchelf)."
    exit 1
fi

# Icon source (rendered to png by svgcvt from the build output)
ICON_SOURCE_SVG="$PROJECT_DIR/uires/svg/logo.svg"
# Icon sizes to generate (following freedesktop.org standards)
ICON_SIZES=(16 32 48 64 128 256)

# Package metadata
PKG_NAME="fundholdings"
PKG_VERSION="1.0.0"
APP_HOME="/usr/local/lib/fundholdings"   # exe + modules + resources + watch list
EXE_NAME="fundholdings"
WATCH_FILE="fundholdings_watch.txt"

# Default watch list shipped with the package. Registered as a conffile so
# dpkg never overwrites the user's own funds on upgrade.
DEFAULT_WATCH_CODES=(
    161725
    005827
    110022
    003096
    000001
    110011
    161005
    260108
    320007
    519674
)

# Runtime modules actually required by this application. The authoritative
# list is generated by CMake into <build>/bin/fundholdings-runtime.list
# (FUNDHOLDINGS_RUNTIME_LIBS in demos/fundholdings/CMakeLists.txt) and copied
# next to the exe by a POST_BUILD step. This array is only a fallback for
# build trees made before that manifest existed.
#
# Explicitly NOT packaged (root CMake builds them for other demos):
#   libws.so, libnetwork.so, librender-gdi.so, libresprovider-7zip.so,
#   libresprovider-zip.so, libscriptmodule-lua.so, libsipcobject.so
# No system-resource zip is shipped on any platform.
FALLBACK_MODULES=(
    libsoui4.so
    libswinx.so
    libcairo.so
    libutilities4.so
    libmsftedit.so
    libhttpclient.so
    libimgdecoder-stb.so
    liblog4z.so
    librender-skia.so
    libtaskloop.so
    libtranslator.so
)
RUNTIME_MANIFEST_NAME="fundholdings-runtime.list"

# Display target distribution
if [ "$TARGET_DISTRO" != "generic" ]; then
    echo "Target Distribution: $TARGET_DISTRO"
else
    echo "Target Distribution: Generic Linux"
fi
echo ""

# ---------------------------------------------------------------
# Locate the build directory containing the executable
# ---------------------------------------------------------------
# CMAKE_RUNTIME_OUTPUT_DIRECTORY is <build>/bin; multi-config generators
# append /Release or /Debug. --build-dir may also point at the bin dir itself.
BUILD_DIR=""
BIN_DIR=""

if [ -n "$BUILD_DIR_OVERRIDE" ]; then
    BUILD_DIR="$BUILD_DIR_OVERRIDE"
    if [ -f "$BUILD_DIR/$EXE_NAME" ]; then
        BIN_DIR="$BUILD_DIR"
    fi
fi

if [ -z "$BIN_DIR" ]; then
    if [ -n "$BUILD_DIR_OVERRIDE" ]; then
        CANDIDATES=("$BUILD_DIR_OVERRIDE")
    else
        CANDIDATES=(
            "$SOUI_ROOT/build_rel"
            "$SOUI_ROOT/build"
            "$SOUI_ROOT/out/build_rel"
            "$SOUI_ROOT/out/build"
            "$PROJECT_DIR/build_rel"
            "$PROJECT_DIR/build"
        )
    fi
    for cand in "${CANDIDATES[@]}"; do
        for sub in bin bin/Release bin/Debug; do
            if [ -f "$cand/$sub/$EXE_NAME" ]; then
                BUILD_DIR="$cand"
                BIN_DIR="$cand/$sub"
                break 2
            fi
        done
    done
fi

if [ -z "$BIN_DIR" ]; then
    echo "Error: could not locate a build directory containing bin/$EXE_NAME"
    echo "Searched (bin, bin/Release, bin/Debug):"
    echo "  $SOUI_ROOT/build_rel"
    echo "  $SOUI_ROOT/build"
    echo "  $SOUI_ROOT/out/build_rel"
    echo "  $SOUI_ROOT/out/build"
    echo "  $PROJECT_DIR/build_rel"
    echo "  $PROJECT_DIR/build"
    echo ""
    echo "Build the project first, then pass the build dir explicitly:"
    echo "  $0 --build-dir=/path/to/build_rel"
    exit 1
fi

if [ ! -f "$BIN_DIR/$EXE_NAME" ]; then
    echo "Error: $EXE_NAME not found at $BIN_DIR/$EXE_NAME"
    exit 1
fi
echo "Build directory : $BUILD_DIR"
echo "Binary directory: $BIN_DIR"

# svgcvt lives next to the exe in the build output (built when SOUI_ENABLE_SVG=ON)
SVG2PNG_TOOL="$BIN_DIR/svgcvt"

# ---------------------------------------------------------------
# Detect build flavor and collect the module list
# ---------------------------------------------------------------
MODE="static"
if [ -f "$BIN_DIR/libsoui4.so" ]; then
    MODE="dll"
fi

# Read the whitelist produced by CMake; never sweep every lib*.so in BIN_DIR.
RUNTIME_MANIFEST=""
for cand in \
    "$BIN_DIR/$RUNTIME_MANIFEST_NAME" \
    "$BUILD_DIR/bin/$RUNTIME_MANIFEST_NAME" \
    "$BUILD_DIR/$RUNTIME_MANIFEST_NAME"
do
    if [ -f "$cand" ]; then
        RUNTIME_MANIFEST="$cand"
        break
    fi
done

WANTED=()
if [ -n "$RUNTIME_MANIFEST" ]; then
    echo "Runtime module list: $RUNTIME_MANIFEST"
    while IFS= read -r mod || [ -n "$mod" ]; do
        mod="${mod%%#*}"
        mod="${mod//[[:space:]]/}"
        case "$mod" in
            lib*.so) WANTED+=("$mod") ;;
        esac
    done < "$RUNTIME_MANIFEST"
else
    echo "Note: $RUNTIME_MANIFEST_NAME not found; using the built-in fallback list."
    echo "      Re-run CMake to generate the authoritative manifest."
    WANTED=("${FALLBACK_MODULES[@]}")
fi

MODULES=()
MISSING=()
for mod in "${WANTED[@]}"; do
    if [ -e "$BIN_DIR/$mod" ]; then
        MODULES+=("$mod")
        # Keep versioned siblings / symlinks belonging to the same library.
        for extra in "$BIN_DIR/$mod".*; do
            [ -e "$extra" ] && MODULES+=("$(basename "$extra")")
        done
    else
        MISSING+=("$mod")
    fi
done

echo ""
if [ "$MODE" = "dll" ]; then
    echo "Detected DLL build (libsoui4.so found)."
    if [ "${#MISSING[@]}" -gt 0 ]; then
        echo "Error: required module(s) not found in $BIN_DIR:"
        for mod in "${MISSING[@]}"; do echo "  - $mod"; done
        echo "Rebuild with the default DLL configuration, or check the manifest."
        exit 1
    fi
else
    echo "Detected static build (no libsoui4.so). Packaging the executable"
    echo "plus the whitelisted component libraries that exist."
    for mod in "${MISSING[@]}"; do
        echo "  - $mod (static build, skipped)"
    done
fi

if [ "${#MODULES[@]}" -eq 0 ]; then
    echo "No runtime modules to package (fully static build)."
else
    for mod in "${MODULES[@]}"; do
        echo "  + $mod"
    done
fi
echo ""

# ---------------------------------------------------------------
# Resolve on-disk resources required at runtime
# ---------------------------------------------------------------
# cmake copies uires/ next to the exe via POST_BUILD. Fall back to the source
# tree so a package can still be produced from a build that skipped that step.
UIRES_SRC="$BIN_DIR/uires"
if [ ! -d "$UIRES_SRC" ]; then
    UIRES_SRC="$PROJECT_DIR/uires"
fi
if [ ! -f "$UIRES_SRC/uires.idx" ]; then
    echo "Error: uires/uires.idx not found (looked in $BIN_DIR/uires and $PROJECT_DIR/uires)"
    exit 1
fi

echo "UI resources  : $UIRES_SRC"
echo "System theme  : none (uires provides all skins; no zip on any platform)"
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
# Copy executable, modules and runtime resources
# ---------------------------------------------------------------
echo "Copying executable..."
cp "$BIN_DIR/$EXE_NAME" "$PACKAGE_DIR$APP_HOME/$EXE_NAME"
chmod 755 "$PACKAGE_DIR$APP_HOME/$EXE_NAME"

if [ "${#MODULES[@]}" -gt 0 ]; then
    echo "Copying SOUI4 module libraries..."
    for mod in "${MODULES[@]}"; do
        cp -a "$BIN_DIR/$mod" "$PACKAGE_DIR$APP_HOME/$mod"
        [ -L "$PACKAGE_DIR$APP_HOME/$mod" ] || chmod 755 "$PACKAGE_DIR$APP_HOME/$mod"
    done

    # Patch RPATH so the exe and libraries also work when launched directly
    # (without the launcher script). $ORIGIN resolves to $APP_HOME itself.
    if command -v patchelf >/dev/null 2>&1; then
        echo "Patching RPATH (\$ORIGIN) with patchelf..."
        patchelf --set-rpath '$ORIGIN' "$PACKAGE_DIR$APP_HOME/$EXE_NAME" || true
        for mod in "${MODULES[@]}"; do
            lib="$PACKAGE_DIR$APP_HOME/$mod"
            [ -L "$lib" ] && continue
            patchelf --set-rpath '$ORIGIN' "$lib" || true
        done
    else
        echo "Note: patchelf not found, skipping RPATH patch."
        echo "      The launcher script still works via LD_LIBRARY_PATH."
    fi
fi

echo "Copying runtime resources..."
mkdir -p "$PACKAGE_DIR$APP_HOME/uires"
cp -a "$UIRES_SRC/." "$PACKAGE_DIR$APP_HOME/uires/"

# ---------------------------------------------------------------
# Default watch list (dpkg conffile)
# ---------------------------------------------------------------
echo "Creating default watch list..."
{
    echo "# fundholdings watch list (one 6-digit code per line)"
    for code in "${DEFAULT_WATCH_CODES[@]}"; do
        echo "$code"
    done
} > "$PACKAGE_DIR$APP_HOME/$WATCH_FILE"
chmod 666 "$PACKAGE_DIR$APP_HOME/$WATCH_FILE"

# Mark it as a conffile: on upgrade dpkg keeps the user's own fund list
# instead of silently overwriting it with the defaults from the package.
echo "$APP_HOME/$WATCH_FILE" > "$PACKAGE_DIR/DEBIAN/conffiles"
chmod 644 "$PACKAGE_DIR/DEBIAN/conffiles"

# ---------------------------------------------------------------
# Launcher script /usr/local/bin/fundholdings
# ---------------------------------------------------------------
echo "Creating launcher script..."
cat > "$PACKAGE_DIR/usr/local/bin/$PKG_NAME" << EOF
#!/bin/bash
# Launcher for $EXE_NAME (SOUI4 Fund Holdings Assistant)
APP_HOME="$APP_HOME"
export LD_LIBRARY_PATH="\$APP_HOME\${LD_LIBRARY_PATH:+:\$LD_LIBRARY_PATH}"
exec "\$APP_HOME/$EXE_NAME" "\$@"
EOF
chmod 755 "$PACKAGE_DIR/usr/local/bin/$PKG_NAME"

# ---------------------------------------------------------------
# Generate application icons from SVG
# ---------------------------------------------------------------
# Prefer svgcvt (nanosvg, same renderer as the app), then common SVG tools.
# A missing converter is not fatal: the desktop entry simply omits Icon=.
render_icon() {
    local src="$1" dst="$2" size="$3"
    rm -f "$dst"
    if [ -x "$SVG2PNG_TOOL" ]; then
        "$SVG2PNG_TOOL" -i "$src" -o "$dst" -s "$size" >/dev/null 2>&1 && [ -s "$dst" ] && return 0
    fi
    if command -v rsvg-convert >/dev/null 2>&1; then
        rsvg-convert -w "$size" -h "$size" -o "$dst" "$src" >/dev/null 2>&1 && [ -s "$dst" ] && return 0
    fi
    if command -v inkscape >/dev/null 2>&1; then
        inkscape "$src" -w "$size" -h "$size" -o "$dst" >/dev/null 2>&1 && [ -s "$dst" ] && return 0
    fi
    # Only accept ImageMagick's convert: Windows also ships a convert.exe
    # (FAT/NTFS disk utility) which would otherwise be picked up here.
    if command -v convert >/dev/null 2>&1 && convert -version 2>/dev/null | grep -qi imagemagick; then
        convert -background none -resize "${size}x${size}" "$src" "$dst" >/dev/null 2>&1 && [ -s "$dst" ] && return 0
    fi
    rm -f "$dst"
    return 1
}

ICON_GENERATED=0
echo "Generating application icons from SVG..."
if [ -f "$ICON_SOURCE_SVG" ]; then
    for size in "${ICON_SIZES[@]}"; do
        icon_dir="$PACKAGE_DIR/usr/local/share/icons/hicolor/${size}x${size}/apps"
        icon_file="$icon_dir/$PKG_NAME.png"
        if render_icon "$ICON_SOURCE_SVG" "$icon_file" "$size" && [ -f "$icon_file" ]; then
            echo "  OK ${size}x${size}"
            ICON_GENERATED=1
        else
            echo "  Failed to generate ${size}x${size} icon"
        fi
    done
    if [ "$ICON_GENERATED" = "1" ]; then
        echo "Icons generated; tool used: svgcvt or an available SVG converter."
    else
        echo "Warning: no SVG converter available (svgcvt/rsvg-convert/inkscape/convert)."
        echo "         The package will be built without an application icon."
    fi
else
    echo "Warning: SVG icon file not found at $ICON_SOURCE_SVG"
    echo "         The package will be built without an application icon."
fi

# ---------------------------------------------------------------
# DEBIAN/control
# ---------------------------------------------------------------
# Runtime dependencies on Linux:
#   - swinx links dl, xcb, uuid, atomic, libstdc++ and optionally ALSA
#   - render-skia needs OpenGL; fontconfig/freetype come from the base system
#   - httpclient is linked against a bundled static libcurl, but HTTPS needs
#     the system CA bundle (ca-certificates)
#   - CJK fonts are recommended for the Chinese UI
case "$TARGET_DISTRO" in
    deepin)
        DEPENDS="libc6 (>= 2.28), libstdc++6, libgcc-s1 | libgcc1, libx11-6, libxcb1, libgl1, libasound2, libuuid1, libfontconfig1, zlib1g, ca-certificates"
        echo "Using Deepin-specific dependencies"
        ;;
    ubuntu)
        DEPENDS="libc6 (>= 2.31), libstdc++6, libgcc-s1 | libgcc1, libx11-6, libxcb1, libgl1, libasound2, libuuid1, libfontconfig1, zlib1g, ca-certificates"
        echo "Using Ubuntu-optimized dependencies"
        ;;
    *)
        DEPENDS="libc6 (>= 2.17), libstdc++6, libgcc-s1 | libgcc1, libx11-6, libxcb1, libgl1, libasound2, libuuid1, libfontconfig1, zlib1g, ca-certificates"
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
Section: utils
Priority: optional
Architecture: $ARCH
Depends: $DEPENDS
Recommends: fonts-noto-cjk | fonts-wqy-microhei | fonts-wqy-zenhei
Maintainer: SOUI Team <setoutsoft@qq.com>
Homepage: https://gitee.com/setoutsoft/soui
Description: Fund holdings analysis tool based on the SOUI4 framework
 fundholdings is a desktop fund holdings / intraday estimate assistant built
 with the SOUI4 Direct UI framework (SOUI5, 5.x). It fetches public quote,
 holding and index data from Eastmoney and renders them in a dark dashboard.
 .
 Key Features:
 - Watch list of funds with a scrollable/fling-enabled tab bar
 - Holdings breakdown, contribution ranking and valuation trend charts
 - Intraday estimate with a simulated-market switch
 - Realtime overview of the three major A-share indexes
 - Empty state and a built-in SOUI5 introduction page
 .
 Technical Highlights:
 - Declarative XML layouts and skins bundled in uires (never a zip package)
 - Background HTTP tasks via ITaskLoop, UI updates via SNotifyCenter
 - Resources are loaded from the application directory (GetAppDir)
EOF

# ---------------------------------------------------------------
# DEBIAN/postinst
# ---------------------------------------------------------------
# Keep APP_HOME / WATCH_FILE in sync with the variables above.
echo "Creating post-installation script..."
cat > "$PACKAGE_DIR/DEBIAN/postinst" << 'EOF'
#!/bin/bash
set -e

# The app reads/writes fundholdings_watch.txt and creates log/ in its
# application directory (SApplication::GetAppDir()). Make sure a regular
# (non-root) user can persist the watch list and write logs.
APP_HOME="/usr/local/lib/fundholdings"
WATCH_FILE="$APP_HOME/fundholdings_watch.txt"
if [ -d "$APP_HOME" ]; then
    if [ ! -f "$WATCH_FILE" ]; then
        printf '# fundholdings watch list (one 6-digit code per line)\n' > "$WATCH_FILE"
    fi
    chmod 666 "$WATCH_FILE" || true
    mkdir -p "$APP_HOME/log" 2>/dev/null || true
    chmod 777 "$APP_HOME/log" 2>/dev/null || true
fi

# Update desktop database if available
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database /usr/local/share/applications || true
fi

# Update icon cache if available
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f /usr/local/share/icons/hicolor || true
fi

echo "fundholdings has been successfully installed!"
echo "Launch it from the applications menu or by running 'fundholdings'."
echo "Your fund watch list is stored in $WATCH_FILE."

exit 0
EOF
chmod 755 "$PACKAGE_DIR/DEBIAN/postinst"

# ---------------------------------------------------------------
# DEBIAN/prerm
# ---------------------------------------------------------------
# The watch list is a conffile: dpkg decide whether to remove it on purge.
# Never delete user data here.
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
ICON_LINE=""
if [ "$ICON_GENERATED" = "1" ]; then
    ICON_LINE="Icon=$PKG_NAME"
fi
cat > "$PACKAGE_DIR/usr/local/share/applications/$PKG_NAME.desktop" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=Fund Holdings
Name[zh_CN]=基金持仓分析
GenericName=Fund Analysis Tool
GenericName[zh_CN]=基金持仓分析工具
Comment=Fund holdings, intraday estimates and A-share index overview
Comment[zh_CN]=基金持仓、盘中估值与大盘指数速览
Exec=$PKG_NAME
$ICON_LINE
Terminal=false
Categories=Office;Finance;
Keywords=fund;holdings;finance;investment;基金;持仓;
StartupNotify=true
StartupWMClass=$PKG_NAME
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
    "$PACKAGE_DIR/DEBIAN/conffiles"
    "$PACKAGE_DIR$APP_HOME/$EXE_NAME"
    "$PACKAGE_DIR$APP_HOME/uires/uires.idx"
    "$PACKAGE_DIR$APP_HOME/$WATCH_FILE"
    "$PACKAGE_DIR/usr/local/bin/$PKG_NAME"
    "$PACKAGE_DIR/usr/local/share/applications/$PKG_NAME.desktop"
)
for file in "${REQUIRED_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo "Error: Required file not found: $file"
        exit 1
    fi
    echo "  OK $file"
done

for mod in "${MODULES[@]}"; do
    if [ ! -e "$PACKAGE_DIR$APP_HOME/$mod" ]; then
        echo "Error: Required module not found in package: $mod"
        exit 1
    fi
    echo "  OK $APP_HOME/$mod"
done

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
echo "====================================="
echo "Package built successfully!"
echo "Output: $OUTPUT_DEB"
echo "====================================="
echo ""
echo "Runtime modules come from $RUNTIME_MANIFEST_NAME; unused components are"
echo "excluded. No system-resource zip is used on any platform."
echo ""
echo "To install the package, run:"
echo "  sudo dpkg -i $OUTPUT_DEB"
echo ""
echo "To remove the package, run:"
echo "  sudo dpkg -r $PKG_NAME"
echo ""
