# Reproducible Teensy 4.0 build

The build wrapper requires the audited toolchain and refuses substitutions:

- Arduino CLI 1.5.1, binary SHA-256 `cbb47ec4742ee49854031728a0eaeb678a2b36d7a797ef833a1a5b02062149c6`
- Teensy core 1.62.0
- `teensy-compile` 15.2.1
- Teensy tools, discovery, and monitor 1.62.0
- FQBN `teensy:avr:teensy40:usb=serial,speed=600,opt=o2std,keys=en-us`
- Audio, Bounce2, ILI9341_t3, sketch-local Encoder2, and its Encoder utility headers from this repository

Choose one UTC value once and retain it with the build output. From a clean checkout:

```bash
ARDUINO_CLI_BIN=/absolute/path/to/arduino-cli \
ARDUINO_DATA_DIRECTORY=/absolute/path/to/arduino-data \
tools/reproducible-build/build-teensy40.sh 20260921T031500Z /absolute/output/directory
```

The wrapper performs two builds from fresh source exports at the same absolute paths and with the same generated `BuildIdentity.h`. It fails unless the two Intel HEX files are byte-identical. Each run retains the generated header, source/tool manifest, complete verbose command log, build directory, exported ELF/HEX and Teensy map/list/symbol artifacts, and hashes.
