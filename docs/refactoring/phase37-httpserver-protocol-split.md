# Phase 37: HttpServer Protocol Split

## Background
`HttpServer.cpp` mixed two concerns:
- socket lifecycle / accept loop / handler dispatch
- HTTP request parsing and response serialization

The file exceeded the project’s single-responsibility target and made protocol-level changes risky.

## Goal
- Separate transport loop from HTTP protocol handling.
- Keep behavior unchanged while lowering per-file complexity.

## Changes
1. Kept transport lifecycle in:
   - `MouseFx/Server/HttpServer.cpp`
   - startup, bind/listen, accept loop, client dispatch
2. Moved protocol parsing/serialization into:
   - `MouseFx/Server/HttpServer.Protocol.cpp`
   - `ParseRequest`
   - `SendResponse`
   - protocol-local helpers (`StatusText`, `SockSendAll`)
3. Updated project metadata:
   - `MFCMouseEffect.vcxproj` add `HttpServer.Protocol.cpp`
   - `MFCMouseEffect.vcxproj.filters` add `HttpServer.Protocol.cpp`

## Validation
- Build command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - `0 warning / 0 error`

## Impact
- `HttpServer` now has clearer internal boundaries:
  - transport control path
  - HTTP protocol path
- Future request/response policy changes can be implemented with lower regression risk.
