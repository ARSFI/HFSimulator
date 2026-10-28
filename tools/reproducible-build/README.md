# Reproducible Teensy 4.0 build

The build wrapper requires the audited toolchain and refuses substitutions:

- Arduino CLI 1.5.1, binary SHA-256 `cbb47ec4742ee49854031728a0eaeb678a2b36d7a797ef833a1a5b02062149c6`
- Teensy core 1.62.0
- `teensy-compile` 15.2.1
- Teensy tools, discovery, and monitor 1.62.0
- FQBN `teensy:avr:teensy40:usb=serial,speed=600,opt=o2std,keys=en-us`
- Audio from `src/libraries`, and Bounce2, ILI9341_t3n, and Encoder2 (including its utility headers) from `src/HFSim_BFD_2_06/libraries`

Choose one UTC value once and retain it with the build output. From a clean checkout:

```bash
ARDUINO_CLI_BIN=/absolute/path/to/arduino-cli \
ARDUINO_DATA_DIRECTORY=/absolute/path/to/arduino-data \
ARDUINO_CLI_CONFIG_FILE=/absolute/path/to/arduino-cli.yaml \
tools/reproducible-build/build-teensy40.sh 20260921T031500Z /absolute/output/directory
```

The wrapper performs two builds from fresh source exports at the same absolute paths and with the same generated `BuildIdentity.h`. It fails unless the two Intel HEX files are byte-identical. Each run retains the generated header, source/tool manifest, complete verbose command log, build directory, exported ELF/HEX and Teensy map/list/symbol artifacts, and hashes.

The optional `ARDUINO_CLI_CONFIG_FILE` selects the CLI configuration without changing the pinned executable. Temporary exports and builds use `LANE_SCRATCH`, then `TMPDIR`, then `/dev/shm`. The production sketch is `src/HFSim_BFD_2_06/HFSim_BFD_2_06.ino`; the Teensy 4.1 alternate is `hardware/Alternate hardware platforms/src/HFSim_BFD_2_06_Proto/HFSim_BFD_2_06_Proto.ino`.
