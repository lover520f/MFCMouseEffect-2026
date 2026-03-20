#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../../.." && pwd)"

source "$repo_root/tools/platform/regression/lib/common.sh"
source "$repo_root/tools/platform/regression/lib/build.sh"
source "$repo_root/tools/platform/manual/lib/macos_core_host.sh"

usage() {
    cat <<'EOF'
Usage:
  tools/platform/package/build-macos-portable.sh [options]

Options:
  --build-dir <path>      Build directory containing mfx_entry_posix_host (default: build-macos)
  --output-dir <path>     Output directory for packaged folder/zip (default: Install/macos)
  --package-name <name>   Package folder base name (default: MFCMouseEffect-macos-arm64-portable)
  --skip-build            Skip rebuilding mfx_entry_posix_host
  --skip-webui-build      Skip rebuilding WebUIWorkspace assets
  --no-zip                Do not create the final zip archive
  --no-dmg                Do not create the final dmg image
  -h, --help              Show this help
EOF
}

build_dir="$repo_root/build-macos"
output_dir="$repo_root/Install/macos"
package_name="MFCMouseEffect-macos-arm64-portable"
app_name="MFCMouseEffect.app"
skip_build=0
skip_webui_build=0
create_zip=1
create_dmg=1

while [[ $# -gt 0 ]]; do
    case "$1" in
    --build-dir)
        [[ $# -ge 2 ]] || { echo "missing value for --build-dir" >&2; exit 1; }
        build_dir="$2"
        shift 2
        ;;
    --output-dir)
        [[ $# -ge 2 ]] || { echo "missing value for --output-dir" >&2; exit 1; }
        output_dir="$2"
        shift 2
        ;;
    --package-name)
        [[ $# -ge 2 ]] || { echo "missing value for --package-name" >&2; exit 1; }
        package_name="$2"
        shift 2
        ;;
    --skip-build)
        skip_build=1
        shift
        ;;
    --skip-webui-build)
        skip_webui_build=1
        shift
        ;;
    --no-zip)
        create_zip=0
        shift
        ;;
    --no-dmg)
        create_dmg=0
        shift
        ;;
    -h|--help)
        usage
        exit 0
        ;;
    *)
        echo "unknown argument: $1" >&2
        exit 1
        ;;
    esac
done

if [[ "$OSTYPE" != darwin* ]]; then
    echo "this packaging script is macOS-only" >&2
    exit 1
fi

webui_dir="$repo_root/MFCMouseEffect/WebUI"
assets_dir="$repo_root/MFCMouseEffect/Assets"
pet_source_dir="$assets_dir/Pet3D/source"
plugin_dist_dir="$repo_root/examples/wasm-plugin-template/dist"
plugin_target_name="demo.template.default.v2"
pet_glb_source="$pet_source_dir/pet-main.glb"
pet_joint_metadata_temp="$(mktemp "/tmp/mfx-pet-joints.XXXXXX.json")"
iconset_temp_dir="$(mktemp -d "/tmp/mfx-appicon.XXXXXX.iconset")"
dmg_stage_dir=""
dmg_rw_temp=""
dmg_mount_point=""
cleanup() {
    rm -f "${pet_joint_metadata_temp:-}"
    rm -rf "${iconset_temp_dir:-}"
    if [[ -n "${dmg_mount_point:-}" ]] && mount | grep -q "on ${dmg_mount_point} "; then
        hdiutil detach "$dmg_mount_point" -force >/dev/null 2>&1 || true
    fi
    rm -f "${dmg_rw_temp:-}"
    if [[ -n "${dmg_stage_dir:-}" ]]; then
        rm -rf "$dmg_stage_dir"
    fi
}
trap cleanup EXIT

mfx_manual_prepare_core_host_binary "$repo_root" "$build_dir" "$skip_build" "$skip_webui_build"
host_bin="$MFX_MANUAL_HOST_BIN"

[[ -x "$host_bin" ]] || { echo "host binary missing: $host_bin" >&2; exit 1; }
[[ -f "$webui_dir/index.html" ]] || { echo "WebUI assets missing: $webui_dir/index.html" >&2; exit 1; }
[[ -f "$pet_source_dir/pet-main.usdz" ]] || { echo "pet asset missing: $pet_source_dir/pet-main.usdz" >&2; exit 1; }
[[ -f "$pet_glb_source" ]] || { echo "pet skeleton metadata source missing: $pet_glb_source" >&2; exit 1; }
[[ -f "$pet_source_dir/pet-actions.json" ]] || { echo "pet action library missing: $pet_source_dir/pet-actions.json" >&2; exit 1; }
[[ -f "$pet_source_dir/pet-appearance.json" ]] || { echo "pet appearance profile missing: $pet_source_dir/pet-appearance.json" >&2; exit 1; }
[[ -f "$pet_source_dir/pet-effects.json" ]] || { echo "pet effects profile missing: $pet_source_dir/pet-effects.json" >&2; exit 1; }
[[ -f "$plugin_dist_dir/plugin.json" ]] || { echo "sample plugin manifest missing: $plugin_dist_dir/plugin.json" >&2; exit 1; }
[[ -f "$plugin_dist_dir/effect.wasm" ]] || { echo "sample plugin wasm missing: $plugin_dist_dir/effect.wasm" >&2; exit 1; }

mkdir -p "$output_dir"
package_root="$output_dir/$package_name"
rm -rf "$package_root"
python3 - "$pet_glb_source" "$pet_joint_metadata_temp" <<'PY'
import json, struct, sys
from pathlib import Path

glb_path = Path(sys.argv[1])
out_path = Path(sys.argv[2])
data = glb_path.read_bytes()
if len(data) < 20:
    raise SystemExit("glb too small")
magic, version, length = struct.unpack_from("<III", data, 0)
if magic != 0x46546C67 or version != 2 or length > len(data):
    raise SystemExit("invalid glb header")
json_len, json_type = struct.unpack_from("<II", data, 12)
if json_type != 0x4E4F534A:
    raise SystemExit("missing json chunk")
root = json.loads(data[20:20 + json_len].decode("utf-8"))
nodes = root.get("nodes") or []
skins = root.get("skins") or []
if not nodes or not skins or not skins[0].get("joints"):
    raise SystemExit("missing skin joints")
joints = {int(v) for v in skins[0]["joints"]}
root_joint = int(skins[0].get("skeleton", skins[0]["joints"][0]))

ordered = []
def walk(node_index: int, include_current: bool) -> None:
    if node_index < 0 or node_index >= len(nodes):
        return
    node = nodes[node_index]
    if include_current and node_index in joints:
        name = node.get("name")
        if isinstance(name, str) and name:
            ordered.append(name)
    for child in node.get("children") or []:
        walk(int(child), True)

walk(root_joint, False)
out_path.write_text(
    json.dumps({"joint_descendant_names": ordered}, ensure_ascii=False, separators=(",", ":")),
    encoding="utf-8",
)
PY
app_root="$package_root/$app_name"
app_contents="$app_root/Contents"
app_macos_dir="$app_contents/MacOS"
app_resources_dir="$app_contents/Resources"
app_runtime_root="$app_resources_dir/MFCMouseEffect"
mkdir -p \
    "$app_macos_dir/plugins/wasm" \
    "$app_runtime_root/Assets/Pet3D/source" \
    "$app_runtime_root"

cp "$host_bin" "$app_macos_dir/MFCMouseEffect"
chmod +x "$app_macos_dir/MFCMouseEffect"
strip -x "$app_macos_dir/MFCMouseEffect"

cp -R "$webui_dir" "$app_runtime_root/WebUI"
cp "$pet_source_dir/pet-main.usdz" "$app_runtime_root/Assets/Pet3D/source/pet-main.usdz"
cp "$pet_joint_metadata_temp" "$app_runtime_root/Assets/Pet3D/source/pet-main.joints.json"
cp "$pet_source_dir/pet-actions.json" "$app_runtime_root/Assets/Pet3D/source/pet-actions.json"
cp "$pet_source_dir/pet-appearance.json" "$app_runtime_root/Assets/Pet3D/source/pet-appearance.json"
cp "$pet_source_dir/pet-effects.json" "$app_runtime_root/Assets/Pet3D/source/pet-effects.json"
mkdir -p "$app_macos_dir/plugins/wasm/$plugin_target_name"
cp "$plugin_dist_dir/plugin.json" "$app_macos_dir/plugins/wasm/$plugin_target_name/plugin.json"
cp "$plugin_dist_dir/effect.wasm" "$app_macos_dir/plugins/wasm/$plugin_target_name/effect.wasm"
mkdir -p "$app_runtime_root/theme-catalog"

build_app_icon() {
    local iconset_dir="$iconset_temp_dir"
    local app_icon_path="$app_resources_dir/AppIcon.icns"
    mkdir -p "$iconset_dir"
    swift - "$iconset_dir" <<'SWIFT'
import AppKit
import Foundation

let outputDir = URL(fileURLWithPath: CommandLine.arguments[1], isDirectory: true)
let entries: [(Int, String)] = [
    (16, "icon_16x16.png"),
    (32, "icon_16x16@2x.png"),
    (32, "icon_32x32.png"),
    (64, "icon_32x32@2x.png"),
    (128, "icon_128x128.png"),
    (256, "icon_128x128@2x.png"),
    (256, "icon_256x256.png"),
]

func makeImage(size: Int) -> NSImage {
    let image = NSImage(size: NSSize(width: size, height: size))
    image.lockFocus()

    let bounds = NSRect(x: 0, y: 0, width: size, height: size)
    NSColor(calibratedRed: 0.92, green: 0.96, blue: 1.0, alpha: 1.0).setFill()
    let background = NSBezierPath(roundedRect: bounds.insetBy(dx: 1, dy: 1),
                                  xRadius: CGFloat(size) * 0.22,
                                  yRadius: CGFloat(size) * 0.22)
    background.fill()

    NSColor(calibratedRed: 0.11, green: 0.22, blue: 0.38, alpha: 1.0).setStroke()
    background.lineWidth = max(2.0, CGFloat(size) * 0.04)
    background.stroke()

    let fontSize = CGFloat(size) * 0.34
    let paragraph = NSMutableParagraphStyle()
    paragraph.alignment = .center
    let attrs: [NSAttributedString.Key: Any] = [
        .font: NSFont.systemFont(ofSize: fontSize, weight: .bold),
        .foregroundColor: NSColor(calibratedRed: 0.11, green: 0.22, blue: 0.38, alpha: 1.0),
        .paragraphStyle: paragraph
    ]
    let text = NSString(string: "MFX")
    let textSize = text.size(withAttributes: attrs)
    let textRect = NSRect(
        x: 0,
        y: (CGFloat(size) - textSize.height) / 2.0 - CGFloat(size) * 0.03,
        width: CGFloat(size),
        height: textSize.height
    )
    text.draw(in: textRect, withAttributes: attrs)

    image.unlockFocus()
    return image
}

for (size, name) in entries {
    let image = makeImage(size: size)
    guard let tiff = image.tiffRepresentation,
          let rep = NSBitmapImageRep(data: tiff),
          let png = rep.representation(using: .png, properties: [:]) else {
        fputs("failed to render icon \(name)\n", stderr)
        exit(1)
    }
    try png.write(to: outputDir.appendingPathComponent(name))
}
SWIFT
    iconutil -c icns "$iconset_dir" -o "$app_icon_path" >/dev/null
}

build_app_icon

cat > "$app_contents/Info.plist" <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>en</string>
    <key>CFBundleDisplayName</key>
    <string>MFCMouseEffect</string>
    <key>CFBundleExecutable</key>
    <string>MFCMouseEffect</string>
    <key>CFBundleIdentifier</key>
    <string>com.mousefx.mfcmouseeffect</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleIconFile</key>
    <string>AppIcon</string>
    <key>CFBundleName</key>
    <string>MFCMouseEffect</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>0.1.0</string>
    <key>CFBundleVersion</key>
    <string>0.1.0</string>
    <key>LSMinimumSystemVersion</key>
    <string>13.0</string>
    <key>LSUIElement</key>
    <true/>
</dict>
</plist>
EOF

zip_path="$output_dir/$package_name.zip"
if [[ "$create_zip" -eq 1 ]]; then
    rm -f "$zip_path"
    (
        cd "$output_dir"
        ditto -c -k --sequesterRsrc --keepParent "$package_name" "$zip_path"
    )
fi

dmg_path="$output_dir/$package_name.dmg"
if [[ "$create_dmg" -eq 1 ]]; then
    rm -f "$dmg_path"
    dmg_stage_dir="$(mktemp -d "$output_dir/.dmg-stage.XXXXXX")"
    dmg_rw_temp="$(mktemp "$output_dir/.${package_name}.XXXXXX.dmg")"
    dmg_mount_point="$(mktemp -d "/tmp/${package_name}.mount.XXXXXX")"
    rm -f "$dmg_rw_temp"
    cp -R "$app_root" "$dmg_stage_dir/$app_name"
    osascript <<EOF >/dev/null
set targetFolder to POSIX file "$dmg_stage_dir"
tell application "Finder"
    make new alias file at targetFolder to POSIX file "/Applications" with properties {name:"Applications"}
end tell
EOF
    swift - "$dmg_stage_dir/Applications" <<'SWIFT'
import AppKit
import Foundation

let aliasPath = CommandLine.arguments[1]
let sourceIcon = URL(fileURLWithPath: "/System/Library/CoreServices/CoreTypes.bundle/Contents/Resources/SidebarApplicationsFolder.icns")
guard let image = NSImage(contentsOf: sourceIcon) else {
    fputs("failed to load Applications folder icon\n", stderr)
    exit(1)
}
let ok = NSWorkspace.shared.setIcon(image, forFile: aliasPath, options: [])
guard ok else {
    fputs("failed to assign Applications alias icon\n", stderr)
    exit(1)
}
SWIFT
    hdiutil create \
        -volname "$package_name" \
        -srcfolder "$dmg_stage_dir" \
        -fs APFS \
        -format UDRW \
        "$dmg_rw_temp" >/dev/null
    hdiutil attach "$dmg_rw_temp" -mountpoint "$dmg_mount_point" -nobrowse -noverify >/dev/null
    osascript <<EOF >/dev/null
set targetFolder to POSIX file "$dmg_mount_point"
tell application "Finder"
    open targetFolder
    delay 0.5
    set targetWindow to front Finder window
    set current view of targetWindow to icon view
    set toolbar visible of targetWindow to false
    set statusbar visible of targetWindow to false
    set the bounds of targetWindow to {120, 120, 840, 440}
    set viewOptions to the icon view options of targetWindow
    set arrangement of viewOptions to not arranged
    set icon size of viewOptions to 128
    delay 0.5
    set position of item "${app_name}" of targetWindow to {170, 170}
    set position of item "Applications" of targetWindow to {500, 170}
    update targetFolder without registering applications
    delay 0.5
    close targetWindow
end tell
EOF
    hdiutil detach "$dmg_mount_point" >/dev/null
    dmg_mount_point=""
    hdiutil convert "$dmg_rw_temp" -format UDBZ -o "$dmg_path" >/dev/null
    rm -f "$dmg_rw_temp"
    dmg_rw_temp=""
    rm -rf "$dmg_stage_dir"
    dmg_stage_dir=""
fi

printf 'package_root=%s\n' "$package_root"
if [[ "$create_zip" -eq 1 ]]; then
    printf 'zip_path=%s\n' "$zip_path"
fi
if [[ "$create_dmg" -eq 1 ]]; then
    printf 'dmg_path=%s\n' "$dmg_path"
fi
