![Geode Mods Installer Banner](banner.png)

# Geoode Installer

Small Windows CLI/Open With handler for installing `.geode` mods into Geometry Dash.

## Build

```powershell
cmake -S . -B build
cmake --build build --config Release
```

The executable is created here:

```text
build\Release\geoode.exe
```

## First setup

Double-click `geoode.exe` and paste the full path to `GeometryDash.exe`, or run:

```powershell
.\build\Release\geoode.exe set C:\Users\drean\OneDrive\Desktop\GeometryDash\GeometryDash.exe
.\build\Release\geoode.exe setup
```

`setup` adds the exe folder to the current user's `PATH` and registers `.geode` files in Windows as `Install Into Geode`.
Open a new terminal after `PATH` changes.

## Commands

```text
geoode
geoode set <path-to-GeometryDash.exe>
geoode install <file.geode>
geoode <file.geode>
geoode open
geoode setup
geoode register [default]
geoode path
geoode where
geoode unset
geoode unregister
geoode autorun on
geoode autorun off
```

When autorun is off, installing a `.geode` file shows a Windows notification instead of launching Geometry Dash.
