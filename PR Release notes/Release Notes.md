Pull-Request Release Notes

PR-8  Gate the periodic RMS report on DEBUG (avoid USB stalls with a non-reading host)


PR-7  Rev 2.06: fix flat-fade gains and match serial level to the front panelRelease Notes.txt
#7·by xmutantson was merged 2 days ago Contributor
1
2

PR-5  Make USB control DSP-authoritative and attest firmware builds
#5·by xmutantson was merged 2 weeks ago Contributor
1

PR-4  USB serial state interface: DSP-authoritative ACK, display watchdog, STATUS/LEVEL/HELP
#4·by xmutantson was closed on Sep 20 Contributor
1

PR-3  Draw the mode header once when initializing a non-WGN mode
#3·by xmutantson was closed on Sep 20 Contributor
1

PR-2  Serial channel mode commands never applied the selected mode
#2·by xmutantson was closed on Sep 20 Contributor
1

----------------------

Rev 2.06

PR-8
The main loop prints a calibration report over USB every 20,000 RMS updates, even with DEBUG OFF:

if (intCountrmsMixIQ1234Out == 20000 * (intCountrmsMixIQ1234Out/20000))// only print every 20000 updates

When a controlling program opens the port only to send a command and read the reply, nobody is reading at the moment that report goes out. The Teensy USB serial writer can then wait on an un-drained transmit buffer (up to about 120 ms before its first timeout), and loop() stalls for that long. On a long quiet bench capture we see an isolated disturbance at about 295.7 s after a channel command, which matches the predicted first possible stall from this report under that usage.

This change prints the report only with DEBUG ON, like the other calibration diagnostics:

if (blnDebugAdjustSN && (intCountrmsMixIQ1234Out == 20000 * (intCountrmsMixIQ1234Out/20000))) // only print every 20000 updates

Unchanged: the counter and all RMS calculations, command replies, display, DSP settings, and the DEBUG default (OFF). DEBUG ON keeps its existing meaning, so its diagnostics may still block a non-reading host.

Changes

    src/HFSim_BFD_2_06/HFSim_BFD_2_06.ino and the alternate-hardware HFSim_BFD_2_06_Proto.ino: gate the report on blnDebugAdjustSN.
    tests/test_periodic_rms_report.py: source test that the report is gated on DEBUG in both sketches and that DEBUG is declared once, default OFF. It fails on current master and passes with this change.

Testing

    Full test suite: 39 tests pass (cd tests && python3 -m unittest discover -p 'test_*.py').
    tools/reproducible-build/build-teensy40.sh: two fresh builds produce byte-identical Teensy 4.0 HEX.
    Not yet flashed or measured on hardware; a run with DEBUG OFF and a host that is not reading between commands should no longer show the stall.

-----------------------

PR-7 
Rev 2.06: fix flat-fade gains and match serial level to the front panel

This combines the stray-semicolon fixes and the serial level correction in one Rev 2.06 release, based on 3cd1f7e.

The S:N update timer now guards its block in both sketches. Each eligible simulation update selects either `Fade()` or the steady `AdjustS_N()` call. Removing the preliminary unfaded adjustment closes the window where an audio block could use steady target gains during flat fading. The existing pre-fix capture found that in 40.7% of approximately 2.9 ms blocks. Fade depth 0, the default, and WGN/MPG/MPM/MPP operation without flat fade are unaffected by that audio defect.

The dead TEST3K/TEST6K bandwidth comparisons and redundant doubled semicolons are also removed. Test-mode bandwidth assignment and filter setup remain unconditional.

`INPUT_MVPP` in both `LEVEL` and `STATUS` now reports `int(fltppInputForDisplay)`, the same quantity used by the front-panel level display. The previous roughly 3-second max-hold remains available as `INPUT_MVPP_MAXHOLD`, with its existing one-decimal format. Output-level fields are unchanged. The original September 22 meter fix remains its own commit with its original message, author and tests, adapted to the moved sketch paths.

Both revision macros are `2.06`. The sketch folders/files are `HFSim_BFD_2_06` and `HFSim_BFD_2_06_Proto`, following PR #5's naming convention. Both change logs summarize the two fixes. Bundled libraries move with the production sketch; their contents are unchanged.

This PR also repairs the merged tests and reproducible-build tooling broken by 3cd1f7e's beta directory and display-library changes. All test sketch paths use the 2.06 names. The recipe and README select the bundled `ILI9341_t3n`, Bounce2 and Encoder2, including Encoder2's existing utility headers, and retain the repository Audio library. The recipe accepts an explicit CLI config and uses the configured scratch directory for temporary exports/builds.

Validation at c4afa727270b2bb0203a6458043fb55ed29b82d8:


- `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s tests -v`: all 25 tests pass, including the original meter test updates.
- The S:N, empty-statement and focused meter regressions fail on exact 3cd1f7e sketch bytes in both sketches, then pass on this branch. The base fixtures only remap file locations; the firmware bytes are unchanged.
- Two fresh Teensy 4.0 production builds through the repository recipe produce byte-identical HEX files. Proto compiles for Teensy 4.1.
- All four patches apply cleanly to a fresh 3cd1f7e with `git am`, producing the identical release tree.

Build UTC is `20261028T023000Z`, using Arduino CLI 1.5.1, Teensy core 1.62.0 and teensy-compile 15.2.1. HEX SHA-256:

```text
production run 1: 6b2258f11edc59e569c2f163fef84956b6eb440dc91c263ebd172d7d9b4e4187
production run 2: 6b2258f11edc59e569c2f163fef84956b6eb440dc91c263ebd172d7d9b4e4187
Proto:           10cfae4b9043717a22d5b97088c96551b77abd6340045c6e94e91e3f14dc025b
```

One side effect: while actively fading, an operator target change no longer prints its calibration block with DEBUG OFF. The target still takes effect through `Fade()`, whose internal adjustments are quiet. Steady operation still prints on target changes, and DEBUG ON remains available.

Bench verification on a unit is still to do: repeat the audio-block capture, check fade depths/rates and profiles, verify default steady operation and test-mode entry, and compare serial levels with the front panel and max-hold. No post-fix hardware measurement is claimed.

Fixes #6

----------------------


Two more fixes are pushed to this branch, and the bench results for the current head are below.

**New commits (both sketches, with regressions that fail on c4afa72):**
- **21786a7: BUSY acknowledgements reported ERROR after success.** A successful `TONE ON` / `TONE OFF` printed `OK`, then `ACK ... ERROR`.
  - The serial BUSY dispatch never recorded the applied item, but `BusyRequestConfirmed()` requires it.
  - The TONE setters also left the readback state stale, and two readbacks returned a literal `0`.
  - This predates 2.06.
- **e1eb36e: TEST3K / TEST6K reachable over serial.** `ParseSimMode()` stopped before the test modes and compared case-sensitively, so the
  front-panel calibration modes could not be entered or verified remotely.
  - The front-panel entry, frequency and exit paths are now shared functions.
  - New commands: `TEST3K:<300..3300>` and `TEST6K:<300..6300>`.
  - HELP and STATUS report the mode and bandwidth.

Tests: 37/37 pass (25 existing + 12 new). Two Teensy 4.0 production builds are byte-identical; Proto compiles for 4.1. REV stays 2.06.

## Bench: current head e1eb36e

Serial STATUS reports `REV=2.06 BUILD=e1eb36e@20261029T210000Z`. Flashed HEX SHA-256:
b1d38fede29758cea8df2cdcb47332443d0f77e764fdd1664e77c46321e9016c.

**BUSY / TONE**
- `TONE ON:1000` gives `ACK BUSY/TONE ON:1000 ... OK`, and STATUS shows `MODE=BUSY INPUT_MVPP=1008`.
- `TONE OFF:0` gives `... OK`.
- `TONE ON:20`, `TONE ON:abc` and `TONE OFF:1` are rejected with `?` and an ERROR acknowledgement; the item does not move.

**TEST3K / TEST6K**
- `TEST3K:1500` gives `ACK TEST3K Sine:1500 ... OK`. STATUS: `MODE=TEST3K Sine`, `BANDWIDTH_HZ=3000`, `OFFSET_HZ=0`, `INPUT_MVPP=455`, outputs
  L/R 1817.8 / 909.5 mVpp (calibration targets 1800 / 900).
- `TEST6K:3000` gives `... OK`. STATUS: `BANDWIDTH_HZ=6000`, outputs L/R 907.5 / 1812.6.
- The band edges `TEST3K:3300` and `TEST6K:6300` are accepted; `TEST3K:3400`, `TEST3K:250` and `TEST6K:6400` are rejected.
- `WGN:40` plus `BANDWIDTH:3000` returns to `MODE=WGN`, 3000 Hz.

**Fade profiles** at S:N 25 dB, against a paired WGN control: all four pass. Spread deltas: MPG 15.31, MPM 17.39, MPP 16.17, MPD 14.69 dB.

## Bench: the 2.06 fade fix (measured on c4afa72; the new commits do not touch the fade or S:N update path)

Fade-profile checks on c4afa72 also passed: MPG 16.70, MPM 15.58, MPP 17.15, MPD 14.64 dB.

**The original 40.7% method.**
- It classified 35-sample windows (0.729 ms) inside fade troughs; approximately 2.9 ms was its persistence lag.
- On a new WGN:25 / depth 20 / rate 1 capture, the unchanged classifier gives 2,199/45,596 = 4.823%.
- A smooth synthetic control with no gain overwrites gives 2,042/45,640 = 4.474%, so the residual is a classifier/envelope effect.

**Separate check.** An approximately 2.9 ms acoustic-block check finds:
- 0/7,449 eligible blocks compatible with the steady-noise range at the original setting;
- 0/65,070 across six WGN depth 20/40 rate combinations plus MPG 20/1.

Eligibility requires phase-average noise at least 12 dB above the steady mean, with the steady range taken from 19,301 control blocks.
This supports the fix acoustically; it does not directly count ISR gain states.

Measured on WGN:25, one 56-second analysis per setting:

| Depth label dB | Rate label Hz | Measured rate Hz | Inferred S:N excursion dB |
|---:|---:|---:|---:|
| 10 | 0.1 | 0.09997 | 9.10 |
| 10 | 1 | 1.00001 | 9.03 |
| 10 | 5 | 4.99994 | 9.04 |
| 20 | 0.1 | 0.09996 | 19.02 |
| 20 | 1 | 0.99997 | 19.00 |
| 20 | 5 | 4.99993 | 19.06 |
| 40 | 0.1 | 0.10000 | 39.01 |
| 40 | 1 | 0.99999 | 38.99 |
| 40 | 5 | 4.99994 | 39.01 |

- The approximately 9/19/39 dB excursions match integer truncation of the 10/20/40 dB cosine law.
- MPG:25 / depth 20 / rate 1 gives 19.08 dB at 1.00002 Hz.
- The depth 20 / rate 0 control is 0.013 dB from depth-zero steady power.

**Serial meter.**
- With the continuous internal 1 kHz tone, eight STATUS reads give `INPUT_MVPP=1008`; max-hold settles at 1008.4, and LEVEL agrees.
- Five 1 kHz / -20 dBFS external playbacks give post-tone max-holds of 286.3-291.6 mVpp.
- STATUS requests queue behind playback on this setup, so they could not sample the display quantity during external playback.

The unit is back at WGN:40, depth/rate zero, unity gains, 3000 Hz bandwidth.

----------------------

Rev 2.05


## Why

The USB serial side had never been seriously exercised by scripted control. Based on correspondence with the Winlink team, this is the first systematic scripted use of that interface.

That exposed three correctness failures in released behavior:

- serial profile commands changed S:N and redrew the panel but did not select the DSP profile;
- a convincing panel redraw could therefore show a mode that had never been applied;
- acknowledgments were emitted before the DSP transition completed.

This is test equipment. If the commanded channel and the live channel can differ, every downstream measurement can be mislabeled. The interface must report what the DSP is actually running, not what the parser or display intended.

This consolidated change supersedes PRs #2, #3, and #4. It incorporates their useful work, corrects the ancillary-command regression in #4, and adds the missing trust and build-attestation pieces.

## Changes

### Tier 0 — make scripted profile selection work

- Apply WGN, MPG, MPM, MPP, and MPD to the live channel mode.
- Restrict live profile replacement to selectors 0–4.
- Let FADE DEPTH, FADE FREQ, OFFSET, input/output gains, and BANDWIDTH update their parameters without replacing the live profile.
- Preserve boot-to-WGN behavior and do not persist channel mode to EEPROM.

The established nine-command profile-first program now ends with live MPP and mode-3 tap initialization:

```text
MPP:25
FADE DEPTH:0
FADE FREQ:0
OFFSET:0
CH1 IN:1
CH2 IN:1
CH1 OUT:1
CH2 OUT:1
BANDWIDTH:3000
```

### Tier 1 — make state authoritative

- Route panel and serial profile changes through synchronous `ApplyChannelMode()`.
- Complete tap/routing initialization before returning `OK` or the structured ACK.
- Add an applied-generation counter to ACK and STATUS output.
- Make the SIM-to-BUSY command reachable and apply BUSY state before acknowledgment.
- Reject fractional syntax for integer-valued commands before mutation, eliminating contradictory `OK`/`ERROR` replies.
- Make STATUS report the live DSP-driving mode, S:N, multipaths, fade depth/rate, offset, four gain settings, bandwidth, measured input/output levels, applied generation, and build identity.
- Keep LEVEL as the STATUS level subset and HELP as the complete command reference.
- Print the same build identity on the physical boot screen, serial boot banner, and STATUS.

### Tier 2 — retain maintenance behavior

- Restore `RESET` with transmit flush before reset.
- Restore `CODECINIT`, completing codec reinitialization before its response.

### Tier 3 — display and repository cleanup

- Retain PR #3's duplicate-header predicate correction.
- Remove tracked Python bytecode and ignore generated test caches.
- Clarify HELP output for queries, replies, and maintenance verbs.

## Validation` section with

## Validation

- Thirteen source-level regression tests pass against both the production and Proto
  sketches where applicable (eight prior tests plus five new source-filter tests):
  the fade internal call is silent (`blnOperatorSource=false`), both live-loop steady
  calls request the printing source, the print guard is `(blnOperatorSource ||
  blnDebugAdjustSN) && (intDesiredSN_dB != intLastDesiredSN_dB)`, the firehose flag
  defaults off, and the `DEBUG` verb and `STATUS DEBUG=` field remain reachable.
- The production sketch is now `HFSim_BFD_2_05`; its TFT display, serial boot banner,
  HELP banner, and STATUS revision field report Rev 2.05. The Proto sketch follows the
  same revision convention.
- Teensy 4.0 builds use Arduino CLI 1.5.1, Teensy core 1.62.0, `teensy-compile` 15.2.1,
  and explicit `teensy:avr:teensy40:usb=serial,speed=600,opt=o2std,keys=en-us` settings.
  Two fresh builds with the same recorded identity produced byte-identical Intel HEX.

Build identity: `7037038@20260921T061316Z`

HEX SHA-256: `8567d5e2a91e24b358d0dfd674f310f8a2a26f5199bf0792db7f80919cb4961b`

Hardware validation: this exact Rev 2.05 HEX was flashed to the Teensy 4.0 HF
simulator and re-verified over serial and on the audio wire.

- Fail-before / pass-after: on the prior firmware a 12-second standing flat fade (WGN,
  fade depth 20 dB, 1 Hz) emitted 514,102 `AdjustS_N` calibration blocks (151.9 MB of
  serial output). After the fix, a 45-second standing flat fade emitted **zero** blocks
  (0 bytes), and all four interleaved `STATUS` reads parsed cleanly, reporting Rev 2.05,
  `DEBUG=OFF`, `FADE_DEPTH_DB=20`, and `BUILD=7037038@20260921T061316Z`.
- Operator-initiated blocks are preserved: standalone `MPP:25` prints exactly one block;
  a serial S:N change mid-fade (`WGN:30`) prints exactly one block and then goes quiet;
  the RESET boot banner prints exactly one block.
- The boot banner and STATUS report Rev 2.05 and `7037038@20260921T061316Z`; the exact
  nine-command program ends with live MPP and mode-3 tap initialisation.
- The Butler fade-certification gate was re-run on the new build: FADECERT MPG returned
  PASS with all five checks true and `build_matches_expected=true`; the MPG profile
  measured envelope std 7.14 dB and a 23.6 dB p95–p05 span with 5 downward −3 dB
  crossings, while the matched WGN control measured 0.05 dB std with zero crossings.
- RESET re-enumerated cleanly and the device was restored to neutral WGN defaults before
  the Butler lock was released.


----------------------
PR-4

Adds an optional USB serial state interface on top of the existing command set. It is independent of the serial-mode bug fixes (PR #2) and preserves the legacy `OK` / `?` replies.

**What it adds**

- **DSP-authoritative ACK.** After each command, the firmware echoes the live DSP state and ends the reply with `OK` or `ERROR`. The DSP is the single source of truth; the reply is not a blind echo of the request, so a command that fails to take effect reports `ERROR`.
- **DSP-to-display watchdog.** A one-way watchdog with change detection keeps the front-panel display in sync with the DSP state and repaints at least every 30 seconds.
- **STATUS** — reports the live channel mode and current S:N.
- **LEVEL** — reports the measured input level in mVp-p.
- **HELP** (also `?`) — prints the command list with valid ranges, terminated by `END HELP`.

**Scope**

Behavior/feature only. No serial-mode bug fixes, no MPG-header change, no RESET/CODECINIT. Based directly on `master`.

**Verification**

- Python unit test for feature independence and DSP-authoritative behavior passes.
- Both sketches (Teensy 4.0 standard, Teensy 4.1 Proto) compile.

On an unamended `master`, the ACK will correctly surface a mode-selection mismatch as `ERROR`; once PR #2's fixes are merged, the same interface confirms the corrected state as `OK`.


----------------------

PR-3

**Symptom:** after the simulator initializes with a mode other than WGN selected (seen with MPG), the front panel shows the mode and S:N header lines twice - four header rows instead of two. The Lvl and CF lines below are unaffected. The duplicate clears on the next mode or parameter redraw, so it is cosmetic, but it recurs.

**Root cause:** in the mode-screen refresh in loop(), the WGN branch condition is `(intMode == 0) || (! blnInitialized)` - any uninitialized state selects the WGN branch regardless of the actual mode. When initializing into MPG (or MPM/MPD), that branch prints the mode + S:N pair, and the mode-specific branch then prints the same pair again. There is no screen clear or cursor reset between the two branches, so the second copy lands two text rows lower instead of overwriting.

**Fix:** select the WGN branch on the mode alone. Initialization of a non-WGN mode falls through to its own branch and draws the header exactly once; WGN behavior is unchanged (its own initialization path still runs identically).

One line changed; no functional/DSP behavior affected - display only.

----------------------

PR-2

### The defect

Selecting a channel profile over the USB serial port (`WGN:`, `MPG:`, `MPM:`, `MPP:`,
`MPD:`) answers `OK` but does not change the running channel profile. Only the target
S:N number is updated; the simulator keeps whichever profile the front-panel encoder
last selected (or WGN after power-on).

Cause: `ParseSetSimParameter(String strParameter, int intMode)` takes a parameter named
`intMode`, which shadows the global `intMode`. For modes 0-4 it validates the request
and sets `intTargetSN`, but the global mode index is never assigned. The follow-up call
`ParseSetParameter(strParameter, intSerialCmdMode)` has the same parameter shadowing,
so it updates the display text and S:N only. The main loop configures the IQ tap delays
and the Doppler update cadence from the unchanged global `intMode`, so the physical
signal path never switches. Both firmware variants (`src/HFSim_BFD_2_03` and
`hardware/Alternate hardware platforms/.../HFSim_BFD_2_03_Proto`) have the same
dispatch code and the same defect.

### How it was found

Driving the simulator from a Raspberry Pi over USB serial for automated modem testing:

- After a commanded `WGN:40`, the received tone showed slow amplitude fading and
  measured about 18.5 dB SNR in a setup that had previously measured about 43 dB on a
  genuinely clean WGN channel — the profile was still a multipath/fading mode from
  earlier front-panel use.
- After commanded `MPP:9`, independent SNR estimators disagreed by ~12 dB because the
  channel was not actually in the commanded profile.
- The TFT display showed the commanded mode name while the output audibly kept the old
  profile, which is what pointed at the dispatch rather than the signal path.

### The fix

In the successful serial dispatch, assign the global mode and clear `blnInitialized`
before applying the parameter update:

```c
if (intSerialCmdMode < 5)
  {
    intMode = intSerialCmdMode;
    blnInitialized = false;
    ParseSetParameter(strParameter, intSerialCmdMode);
  }
```

Clearing `blnInitialized` makes the next `loop()` pass run the exact mode-entry
initialization a front-panel mode change runs (tap delays, path mixer gains, display),
and the Doppler update cadence follows the global `intMode` from then on. The target
S:N set by the command is preserved. Front-panel behavior, EEPROM handling, and the
BUSY-mode command path are untouched. Applied identically to both firmware variants.

`tests/test_serial_mode_dispatch.py` (plain `python -m unittest
tests.test_serial_mode_dispatch`) guards the dispatch block of both sources: it fails
on the previous code and passes with the fix.

### Validation on hardware

Flashed the fixed firmware to the Teensy 4.0 unit on the same Raspberry Pi bench and
re-ran the failing sequence:

- Commanded `WGN:40`: measured 43.9 dB SNR with a flat envelope (std 0.003 dB, Doppler-band
  energy fraction 0.0005) — the clean channel is actually delivered. Before the fix the same
  command measured 18.5 dB with visible slow fading.
- Commanded `MPP:9`: fading now genuinely present — envelope std 5.3 dB, p95-p05 spread
  17.5 dB, Doppler-band energy fraction 0.79, and the serial debug echo
  `Line 1014 SetIQTapDelays: intMode = 3` confirms the global mode assignment on dispatch.
- Commanded `WGN:40` again: 44.2 dB, flat — live re-switching works in both directions
  without a power cycle.
- The two independent SNR estimators that disagreed by ~38 dB on a commanded-clean channel
  before the fix now agree within 0.13 dB on it.
- Power-cycle behavior unchanged: after `RESET` the unit boots to WGN as before (modes are
  deliberately not persisted), with the same banner and serial number.

---

--------------
