"""The S:N timer owns a mutually exclusive fade/steady gain update."""
import unittest

from firmware_source import FIRMWARES, ROOT, cpp_tokens, delimiter_pairs


def sn_update(source):
    tokens = cpp_tokens(source)
    pairs = delimiter_pairs(tokens)
    guards = []
    for i, (token, _) in enumerate(tokens[:-1]):
        if token == "if" and tokens[i + 1][0] == "(":
            end = pairs[i + 1]
            condition = [t for t, _ in tokens[i + 2:end]]
            if "ulngLastSNUpdateUs" in condition:
                guards.append((condition, end + 1))
    if len(guards) != 1:
        raise AssertionError("expected exactly one elapsed-time S:N guard")
    condition, body = guards[0]
    expected = cpp_tokens("int(ulngCurrentElapsedTimeUs - ulngLastSNUpdateUs) > random(2000, 5000)")
    if condition != [t for t, _ in expected]:
        raise AssertionError("S:N guard must retain the elapsed-time/random interval")
    # Return the following braces even on the broken source, so the gain-write
    # check independently exposes the second defect before the fix.
    real_body = tokens[body][0] == "{"
    brace = body if real_body else body + 1
    if tokens[brace][0] != "{":
        raise AssertionError("missing S:N update block")
    return real_body, [t for t, _ in tokens[brace + 1:pairs[brace]]]


class SNUpdateGuardTest(unittest.TestCase):
    def test_elapsed_time_guard_has_a_real_body(self):
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                real_body, _ = sn_update(firmware.read_text(encoding="utf-8"))
                self.assertTrue(real_body, "empty S:N guard: update runs on every loop")

    def test_fade_and_steady_gains_are_mutually_exclusive(self):
        expected = [t for t, _ in cpp_tokens("""
            ulngLastSNUpdateUs = ulngCurrentElapsedTimeUs;
            if ((intFadeDepth_dB > 0) && (fltFadeRate > .01) && blnSim) {
                Fade(intTargetSN, intFadeDepth_dB, fltFadeRate);
            } else if (blnSim) {
                AdjustS_N(intTargetSN, fltppLPInputMeasAvg, true);
            }
        """)]
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                _, body = sn_update(firmware.read_text(encoding="utf-8"))
                self.assertNotIn("AdjustS_N", body[:body.index("Fade")],
                                 "unfaded gains must not precede Fade()")
                self.assertEqual(body, expected,
                                 "one faded or steady update, gated by blnSim")


if __name__ == "__main__":
    unittest.main()
