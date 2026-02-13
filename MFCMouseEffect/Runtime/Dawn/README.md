# Dawn Runtime Fallback Directory

Place Dawn runtime DLLs here for development fallback:
- webgpu_dawn.dll (preferred)
- dawn_native.dll
- dawn.dll

Runtime load order:
1. Exe directory (packaged/runtime deployment)
2. This fallback directory: MFCMouseEffect/Runtime/Dawn

When packaging/releasing, keep DLLs next to MFCMouseEffect.exe.
