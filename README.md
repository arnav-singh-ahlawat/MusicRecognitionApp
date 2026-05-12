# 🎵 GPU/CPU-Accelerated Music Recognition Engine

A real-time **audio fingerprinting and music recognition system** built in **C++/Qt**, with optional **OpenCL acceleration** for heavy DSP workloads.

---

## ✨ Features
- 🎧 Record audio from microphone or upload `.wav` files.
- ⚡ Real-time **fingerprint extraction** and **song recognition**.
- 🚀 OpenCL acceleration (up to **6x faster** than CPU-only).
- 💾 SQLite database for storing songs and fingerprints.
- 🖥️ Cross-platform Qt Widgets interface.
- 🛡️ CPU fallback: works even without OpenCL drivers.

---

## 🎬 App Demo

https://github.com/user-attachments/assets/351a88d6-7a6c-4d1d-abaf-486839b29ba8

---

## 🛠️ Prerequisites (Windows 11 target environment)

- **CMake** → CLion bundles this.
- **Qt 6.x (MinGW build)** → for reference, I used `C:\Qt\6.9.2\mingw_64\` in my build.
- **MinGW toolchain** → from Qt, CLion or MSYS2.
- **FFmpeg** → to preprocess audio into `.wav` format.
- **Git** → to clone this repo.
- *(Optional)* **OpenCL runtime** → GPU drivers that expose an OpenCL platform.

---

## 🚀 Build Instructions

### 1. Clone the Repository
```bash
git clone https://github.com/arnav-singh-ahlawat/MusicRecognitionApp.git
cd MusicRecognitionApp
```

### 2. Configure the Build

**Using CLion (recommended):**
1. Open the project in CLion.
2. Go to *Settings → Build, Execution, Deployment → Toolchains → MinGW*:
   - C Compiler: `gcc.exe` (from `C:\Qt\6.9.2\mingw_64\bin`)
   - C++ Compiler: `g++.exe` (from the same folder)
   - CMake: bundled
   - Make: ninja (bundled)
3. Now, go to *Settings → Build, Execution, Deployment → CMake*, and in **CMake options**, add:
   ```bash
   -DCMAKE_PREFIX_PATH="C:/Qt/6.9.2/mingw_64/lib/cmake"
   ```

**Manual build with CMake:**
```bash
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.9.2/mingw_64/lib/cmake" -B build
cmake --build build
```

### 3. Enable GPU Acceleration (Optional)
By default, the app runs in CPU-only mode.

To enable GPU acceleration:
```bash
cmake -G "MinGW Makefiles" -DUSE_OPENCL=ON -DCMAKE_PREFIX_PATH="C:/Qt/6.9.2/mingw_64/lib/cmake" -B build
cmake --build build
```
- When `USE_OPENCL=ON`: project links against `OpenCL::OpenCL`, compiles GPU code, and enables `magnitudeBatch()` for magnitude and power spectrum computation.
- When `USE_OPENCL=OFF` (default): GPU code is **not compiled**, and only CPU paths run.

### 4. Run the Application
The built executable will be in `build/` (or `cmake-build-debug-mingw/` if using CLion).

Locate MusicRecognitionApp.exe inside `build`/`cmake-build-debug-mingw`, and run it.

If running fails due to missing Qt DLLs, use the Qt deployment tool while being inside the same directory:
```bash
"C:\Qt\6.9.2\mingw_64\bin\windeployqt.exe" cmake-build-debug-mingw\MusicRecognitionApp.exe
```

---

## 📂 Usage
- **Upload WAV file** → Extract fingerprint + enter metadata → Store in database.
- **Record (For 10s)** → Capture mic input → Recognize against database.
- **Play/Stop** → Playback uploaded audio for testing.
- **Database reset** → Delete `music.db` or run `VACUUM`.

<img width="1280" height="758" alt="Image" src="https://github.com/user-attachments/assets/4bbbb799-6eb4-4313-a658-c22832b11b2d" />

---

## 🗄️ Database Initialization
- On first run, `music.db` (SQLite) is created automatically.
- If missing, schema migration recreates it.
- Safe to delete `music.db` anytime to reset.

---

## 🔊 FFmpeg

This app currently supports audio in only **`.wav` format (PCM16)**.

### Preparing Your Own Tracks

If you want to add your own music for testing, convert `.mp3`/`.aac`/etc. into `.wav` with [FFmpeg](https://ffmpeg.org/):

```bash
ffmpeg -i <input-audio>.mp3 -ar 44100 -ac 1 -sample_fmt s16 -map_metadata -1 <output-audio>.wav
```

* `-ar 44100` → sets sample rate to **44.1kHz**
* `-ac 1` → forces **mono audio** (required for fingerprinting)
* `-sample_fmt s16` → encodes as **16-bit PCM**
* `-map_metadata -1` → strips metadata for consistency

---

## ⚡ Known Limitations
- Supports only `.wav` (PCM16) audio format.
- OpenCL acceleration covers only magnitude and power spectrum computation (not FFT).
- Matching algorithm is simple (vote-based).
- GUI is minimal (basic upload/record/play/stop flow).

---

## 🔧 Potential Improvements
- Add an audio format conversion pipeline (via FFmpeg) to support MP3, AAC, and other formats.
- Implement OpenCL acceleration to cover FFT and additional DSP workloads.
- More advanced recognition algorithm (better scoring, noise resilience).
- GUI improvements (waveform visualization, metadata editing).


---

## 🤝 Open-Source Contribution
PRs are welcome! Please:

- Follow existing code style.
- Comment your code clearly.
- Open an issue for major changes before submitting.

---

## 📜 License
This project is licensed under the MIT License. See the [LICENSE](https://github.com/arnav-singh-ahlawat/MusicRecognitionApp/blob/master/LICENSE.txt) file for details.
