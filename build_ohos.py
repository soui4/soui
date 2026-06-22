#!/usr/bin/env python3
import argparse
import os
import platform
import shutil
import subprocess
import sys
import time
from pathlib import Path


SUPPORTED_OHOS_ARCHES = ("arm64-v8a", "armeabi-v7a", "x86_64")


def default_sdk_root():
    env_value = (
        os.environ.get("HMOS_SDK_PATH")
        or os.environ.get("hmos_sdk_dir")
        or os.environ.get("DEVECO_SDK_HOME")
        or os.environ.get("OHOS_SDK_ROOT")
    )
    if env_value:
        return Path(env_value)

    system = platform.system()
    if system == "Windows":
        sdk_without_spaces = Path("C:/sdk/default")
        if sdk_without_spaces.exists():
            return sdk_without_spaces
        return Path(r"C:\Program Files\Huawei\DevEco Studio\sdk\default")
    if system == "Darwin":
        return Path("/Applications/DevEco-Studio.app/Contents/sdk/default")
    return Path.home() / "command-line-tools" / "sdk" / "default"


def resolve_native_root(sdk_root):
    sdk_root = Path(sdk_root).resolve()
    if (sdk_root / "build" / "cmake" / "ohos.toolchain.cmake").exists():
        return sdk_root
    if sdk_root.name == "native" and sdk_root.parent.name == "hms":
        return sdk_root.parent.parent / "openharmony" / "native"
    return sdk_root / "openharmony" / "native"


def resolve_hms_native_root(sdk_root):
    sdk_root = Path(sdk_root).resolve()
    if (sdk_root / "build" / "cmake" / "hmos.toolchain.cmake").exists():
        return sdk_root
    if sdk_root.name == "native" and sdk_root.parent.name == "openharmony":
        return sdk_root.parent.parent / "hms" / "native"
    return sdk_root / "hms" / "native"


def sdk_tool_name(name):
    return f"{name}.exe" if platform.system() == "Windows" else name


def cmake_path_value(path):
    return str(path).replace("\\", "/")


def quote_cmd(cmd):
    return " ".join(f'"{item}"' if " " in item else item for item in cmd)


def run(cmd, cwd, env=None):
    print("+ " + quote_cmd([str(item) for item in cmd]), flush=True)
    subprocess.run([str(item) for item in cmd], cwd=str(cwd), env=env, check=True)


def remove_tree_with_retry(path, retries=5, delay_seconds=0.5):
    for attempt in range(1, retries + 1):
        try:
            shutil.rmtree(path)
            return
        except PermissionError:
            if attempt == retries:
                if path.exists() and not any(path.iterdir()):
                    print(f"Warning: {path} is empty but locked; continuing.", flush=True)
                    return
                raise
            time.sleep(delay_seconds)


def format_mb(size):
    return f"{size / 1024 / 1024:.2f} MB"


def existing_file(path, label):
    if not path.exists():
        raise FileNotFoundError(f"{label} not found: {path}")
    return path


def parse_args():
    parser = argparse.ArgumentParser(description="Configure and build SOUI for HarmonyOS/OpenHarmony.")
    parser.add_argument("--source-dir", default=".", help="SOUI source directory, default: current directory")
    parser.add_argument("--arch", default="arm64-v8a", choices=SUPPORTED_OHOS_ARCHES, help="OHOS ABI")
    parser.add_argument("--build-dir", default=None, help="Default: <source-dir>/build-ohos-<arch>")
    parser.add_argument("--sdk-root", default=str(default_sdk_root()), help="DevEco sdk/default root or native SDK root")
    parser.add_argument("--build-type", default="Release", choices=("Debug", "Release"), help="CMake build type")
    parser.add_argument("--jobs", default=str(os.cpu_count() or 6), help="Parallel build jobs")
    parser.add_argument("--target", action="append", default=[], help="Build a CMake target. May be passed multiple times")
    parser.add_argument("--clean", action="store_true", help="Remove build directory before configuring")
    parser.add_argument("--configure-only", action="store_true", help="Only run CMake configure/generate")
    parser.add_argument("--no-strip", action="store_true", help="Do not strip .so outputs")
    parser.add_argument("--no-copy", action="store_true", help="Do not copy .so outputs to --copy-dir")
    parser.add_argument("--copy-dir", default=None, help="Default: <source-dir>/output/ohos/<arch>")
    parser.add_argument("--no-hap-copy", action="store_true", help="Do not copy .so outputs to the OHOS demo HAP libs directory")
    parser.add_argument("--hap-libs-dir", default=None, help="Default: <source-dir>/ohos_demo2/entry/libs/<arch>")
    parser.add_argument("--with-http", action="store_true", help="Build httpclient and its dependencies")
    parser.add_argument("--with-tools", action="store_true", help="Build SOUI tools")
    parser.add_argument("--with-demos", action="store_true", help="Build SOUI demos")
    parser.add_argument("--with-games", action="store_true", help="Build SOUI games")
    parser.add_argument("--static-components", action="store_true", help="Build SOUI components as static libraries")
    parser.add_argument(
        "--rc-compiler",
        default="AUTO",
        choices=("AUTO", "MINGW", "LLVMRC", "OFF", "auto", "mingw", "llvmrc", "off"),
        help="Resource compiler backend for OHOS .rc files, default: AUTO",
    )
    parser.add_argument("--extra-cmake-arg", action="append", default=[], help="Extra argument forwarded to CMake configure")
    return parser.parse_args()


def main():
    args = parse_args()
    source_dir = Path(args.source_dir).resolve()
    build_dir = Path(args.build_dir).resolve() if args.build_dir else source_dir / f"build-ohos-{args.arch}"
    sdk_root = Path(args.sdk_root).resolve()
    native_root = resolve_native_root(sdk_root)
    hms_native_root = resolve_hms_native_root(sdk_root)

    toolchain = existing_file(hms_native_root / "build" / "cmake" / "hmos.toolchain.cmake", "HMOS toolchain")
    cmake_tool = existing_file(native_root / "build-tools" / "cmake" / "bin" / sdk_tool_name("cmake"), "SDK cmake")
    ninja_tool = existing_file(native_root / "build-tools" / "cmake" / "bin" / sdk_tool_name("ninja"), "SDK ninja")
    bisheng_bin = hms_native_root / "BiSheng" / "bin"
    cc_tool = existing_file(bisheng_bin / sdk_tool_name("clang"), "BiSheng clang")
    cxx_tool = existing_file(bisheng_bin / sdk_tool_name("clang++"), "BiSheng clang++")
    strip_tool = bisheng_bin / sdk_tool_name("llvm-strip")
    if not args.no_strip:
        existing_file(strip_tool, "llvm-strip")

    if args.clean and build_dir.exists():
        remove_tree_with_retry(build_dir)

    copy_dir = Path(args.copy_dir).resolve() if args.copy_dir else source_dir / "output" / "ohos" / args.arch
    hap_libs_dir = (
        Path(args.hap_libs_dir).resolve()
        if args.hap_libs_dir
        else source_dir / "ohos_demo2" / "entry" / "libs" / args.arch
    )

    env = os.environ.copy()
    env["PATH"] = str(cmake_tool.parent) + os.pathsep + str(bisheng_bin) + os.pathsep + env.get("PATH", "")
    env["HMOS_SDK_PATH"] = str(sdk_root)
    env["hmos_sdk_dir"] = str(sdk_root)
    env["OHOS_SDK_ROOT"] = str(sdk_root)
    env["CC"] = str(cc_tool)
    env["CXX"] = str(cxx_tool)

    cmake_args = [
        cmake_tool,
        "-S",
        source_dir,
        "-B",
        build_dir,
        "-G",
        "Ninja",
        f"-DCMAKE_BUILD_TYPE={args.build_type}",
        "-DCMAKE_SYSTEM_NAME=OHOS",
        f"-DCMAKE_TOOLCHAIN_FILE={cmake_path_value(toolchain)}",
        f"-DCMAKE_MAKE_PROGRAM={cmake_path_value(ninja_tool)}",
        f"-DOHOS_ARCH={args.arch}",
        f"-DCMAKE_OHOS_ARCH_ABI={args.arch}",
        f"-DOHOS_SDK_NATIVE={cmake_path_value(native_root)}",
        f"-DHMOS_SDK_NATIVE={cmake_path_value(hms_native_root)}",
        f"-DCMAKE_C_COMPILER_AP={cmake_path_value(cc_tool)}",
        f"-DCMAKE_CXX_COMPILER_AP={cmake_path_value(cxx_tool)}",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
        "-DSOUI_BUILD_RICHEDIT=ON",
        "-DSOUI_ENABLE_ACC=OFF",
        "-DSOUI_ENABLE_SPY=OFF",
        f"-DSOUI_BUILD_HTTPCLIENT={'ON' if args.with_http else 'OFF'}",
        "-DSOUI_BUILD_WS=OFF",
        f"-DSOUI_BUILD_TOOLS={'ON' if args.with_tools else 'OFF'}",
        f"-DSOUI_BUILD_DEMOS={'ON' if args.with_demos else 'OFF'}",
        f"-DSOUI_BUILD_GAMES={'ON' if args.with_games else 'OFF'}",
        f"-DSOUI_ENABLE_COM_LIB={'ON' if args.static_components else 'OFF'}",
        "-DCMAKE_CXX_FLAGS=-stdlib=libc++ -ffunction-sections -fdata-sections",
        "-DCMAKE_C_FLAGS=-ffunction-sections -fdata-sections",
        "-DCMAKE_SHARED_LINKER_FLAGS=-Wl,--gc-sections -Wl,--exclude-libs,ALL",
        "-DCMAKE_EXE_LINKER_FLAGS=-Wl,--gc-sections -Wl,--exclude-libs,ALL",
        "-DTOOLCHAIN_SUPPORTS_ATTRIBUTE_CONSTRUCTOR_EXITCODE=0",
        "-DTOOLCHAIN_SUPPORTS_ATTRIBUTE_DESTRUCTOR_EXITCODE=0",
        f"-DSOUI_RC_COMPILER={args.rc_compiler.upper()}",
        "--no-warn-unused-cli",
    ]
    cmake_args.extend(args.extra_cmake_arg)
    run(cmake_args, source_dir, env)

    if args.configure_only:
        print(f"Configured OHOS build directory: {build_dir}")
        return

    if args.target:
        for target in args.target:
            run([cmake_tool, "--build", build_dir, "--target", target, "-j", args.jobs], source_dir, env)
    else:
        run([cmake_tool, "--build", build_dir, "-j", args.jobs], source_dir, env)

    outputs = sorted(build_dir.rglob("*.so"))
    if not outputs:
        raise FileNotFoundError(f"No .so output found under: {build_dir}")

    if not args.no_strip:
        for output in outputs:
            before_size = output.stat().st_size
            run([strip_tool, "--strip-unneeded", output], source_dir, env)
            after_size = output.stat().st_size
            print(f"Output: {output}")
            print(f"Before strip: {before_size} bytes ({format_mb(before_size)})")
            print(f"After strip:  {after_size} bytes ({format_mb(after_size)})")
            print(f"Saved:        {before_size - after_size} bytes ({format_mb(before_size - after_size)})")

    if not args.no_copy:
        copy_dir.mkdir(parents=True, exist_ok=True)
        for output in outputs:
            dest = copy_dir / output.name
            shutil.copy2(output, dest)
            print(f"Copied: {output} -> {dest}")

        if not args.no_hap_copy and (source_dir / "ohos_demo2" / "entry").exists():
            hap_libs_dir.mkdir(parents=True, exist_ok=True)
            for old_output in hap_libs_dir.glob("*.so"):
                old_output.unlink()
            for output in outputs:
                dest = hap_libs_dir / output.name
                shutil.copy2(output, dest)
                print(f"Copied for HAP: {output} -> {dest}")


if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as exc:
        sys.exit(exc.returncode)
