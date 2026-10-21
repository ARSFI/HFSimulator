"""Source-pattern guard for the fade serial-flood fix (Rev 2.05).

The AdjustS_N calibration debug output must be classified by source: the fade state
machine's internal per-step S:N adjustment must be SILENT by default (so it cannot
flood the serial link from inside the envelope-shaping loop), while operator-initiated
S:N changes (boot/initial apply, front-panel dial, USB/serial command) still emit one
calibration block on change. An opt-in DEBUG firehose may restore the legacy per-step
trace, but it must default OFF and stay reachable/observable.

These are static-source assertions checked against both the production and Proto
sketches, matching the existing firmware test convention.
"""
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PRODUCTION = ROOT / "src" / "HFSim_BFD_2_05" / "HFSim_BFD_2_05.ino"
PROTO = (
    ROOT
    / "hardware"
    / "Alternate hardware platforms"
    / "src"
    / "HFSim_BFD_2_05_Proto"
    / "HFSim_BFD_2_05_Proto.ino"
)

FIRMWARES = (PRODUCTION, PROTO)


def extract_function_body(source: str, signature_prefix: str) -> str:
    """Return the brace-delimited body of the first function whose line starts with
    signature_prefix (e.g. 'void Fade ('). Brace-count from the first '{' after it."""
    start = source.find(signature_prefix)
    assert start != -1, f"signature not found: {signature_prefix!r}"
    brace = source.find("{", start)
    assert brace != -1, f"no opening brace after {signature_prefix!r}"
    depth = 0
    for i in range(brace, len(source)):
        c = source[i]
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return source[brace : i + 1]
    raise AssertionError(f"unbalanced braces for {signature_prefix!r}")


class AdjustSNSourceFilterTest(unittest.TestCase):
    def test_adjustsn_takes_a_source_classifier(self):
        for fw in FIRMWARES:
            with self.subTest(firmware=str(fw.relative_to(ROOT))):
                src = fw.read_text(encoding="utf-8")
                self.assertIn(
                    "void AdjustS_N (int intDesiredSN_dB, float fltppLPInputMeasAvg, boolean blnOperatorSource)",
                    src,
                    "AdjustS_N must carry an operator-source classifier argument",
                )

    def test_debug_print_is_source_or_firehose_gated(self):
        for fw in FIRMWARES:
            with self.subTest(firmware=str(fw.relative_to(ROOT))):
                src = fw.read_text(encoding="utf-8")
                # The calibration print fires only for an operator source OR the opt-in
                # firehose, and only when the requested S:N actually changed.
                self.assertIn(
                    "if ((blnOperatorSource || blnDebugAdjustSN) && (intDesiredSN_dB != intLastDesiredSN_dB))",
                    src,
                )
                # The pre-fix ungated guard (would fire on every fade step) must be gone.
                self.assertNotIn(
                    "  if (intDesiredSN_dB != intLastDesiredSN_dB)\n", src
                )
                # The intermediate flag-only guard (would also silence operator changes) is gone.
                self.assertNotIn(
                    "if (blnDebugAdjustSN && (intDesiredSN_dB != intLastDesiredSN_dB))", src
                )

    def test_fade_internal_step_is_silent_source(self):
        for fw in FIRMWARES:
            with self.subTest(firmware=str(fw.relative_to(ROOT))):
                src = fw.read_text(encoding="utf-8")
                body = extract_function_body(src, "void Fade (")
                # Fade() adjusts S:N internally; that call must pass the silent source.
                self.assertIn("AdjustS_N (intCurrentFadeSN_dB", body)
                self.assertIn(
                    "AdjustS_N (intCurrentFadeSN_dB , fltppLPInputMeasAvg, false)", body
                )
                self.assertNotIn(
                    "AdjustS_N (intCurrentFadeSN_dB , fltppLPInputMeasAvg, true)", body
                )
                # No AdjustS_N call inside Fade() may request the operator (printing) source.
                self.assertNotIn("fltppLPInputMeasAvg, true)", body)
                # Fade() itself must not directly emit serial output on the per-step path.
                self.assertNotIn("Serial.print", body)

    def test_operator_steady_calls_request_the_printing_source(self):
        for fw in FIRMWARES:
            with self.subTest(firmware=str(fw.relative_to(ROOT))):
                src = fw.read_text(encoding="utf-8")
                # Both live-loop steady-adjust call sites (which pick up operator dial/serial
                # changes to the target S:N) request the operator source.
                self.assertIn(
                    "if (blnSim) { AdjustS_N ( intTargetSN, fltppLPInputMeasAvg, true);}",
                    src,
                )
                self.assertIn(
                    "AdjustS_N ( intTargetSN, fltppLPInputMeasAvg, true);", src
                )
                # No steady call may pass the silent source (that would drop the operator block).
                self.assertNotIn(
                    "AdjustS_N ( intTargetSN, fltppLPInputMeasAvg, false)", src
                )

    def test_firehose_defaults_off_and_stays_reachable(self):
        for fw in FIRMWARES:
            with self.subTest(firmware=str(fw.relative_to(ROOT))):
                src = fw.read_text(encoding="utf-8")
                # Default OFF: a stock boot never floods on a fade.
                self.assertIn("boolean blnDebugAdjustSN = false;", src)
                # Reachable + observable: the toggle verb and the STATUS field both exist.
                self.assertIn('strCmd == "DEBUG ON"', src)
                self.assertIn('strCmd == "DEBUG OFF"', src)
                self.assertIn('Serial.print(" DEBUG=");', src)


if __name__ == "__main__":
    unittest.main()
