# sv-dashboard-gtk (gtkmm / C++ / Debian)

Two-page 5×3 tile dashboard using bundled Font Awesome 6 Free webfonts.

## Debian deps
```bash
sudo apt update
sudo apt install -y \
  meson ninja-build pkg-config \
  libgtkmm-3.0-dev libfontconfig1-dev \
  curl unzip # python3
```

## Fetch FA6 webfonts
```bash
./scripts/fetch-fontawesome.sh
```

## Build & run
```bash
meson setup build
meson compile -C build
./build/sv-dashboard
```

## Icon configuration
The launcher reads YAML config from `~/.config/sv-dashboard-gtk/sv-dashboard.yaml` by default on Linux (or the path in `SV_DASHBOARD_CONFIG`). On Windows it uses `%LOCALAPPDATA%\\sv-dashboard-gtk\\sv-dashboard.yaml`. If that file is missing, it falls back to the packaged `sv-dashboard.yaml` installed under `share/sv-dashboard-gtk/sv-dashboard.yaml` so a default layout is shown. A sample config matching the BBN launcher format is in `assets/sv-dashboard.yaml` for reference/copying. The `fa` field is required and should be a Font Awesome icon name.
