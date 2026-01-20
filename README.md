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
