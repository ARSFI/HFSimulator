"""The periodic RMS calibration report is opt-in DEBUG output.

An unsolicited USB write can wait on a host that is not reading the port,
so the report must not run in normal (DEBUG OFF) operation.
"""
import unittest

from firmware_source import FIRMWARES, ROOT, cpp_tokens, delimiter_pairs


def report_condition(source):
    tokens = cpp_tokens(source)
    pairs = delimiter_pairs(tokens)
    found = []
    for i, (token, _) in enumerate(tokens[:-1]):
        if token == "if" and tokens[i + 1][0] == "(":
            condition = [t for t, _ in tokens[i + 2:pairs[i + 1]]]
            if "intCountrmsMixIQ1234Out" in condition and "20000" in condition:
                found.append(condition)
    if len(found) != 1:
        raise AssertionError("expected exactly one periodic RMS report condition")
    return found[0]


class PeriodicRMSReportTest(unittest.TestCase):
    def test_report_is_gated_on_debug(self):
        expected = [t for t, _ in cpp_tokens(
            "blnDebugAdjustSN && (intCountrmsMixIQ1234Out == 20000 * (intCountrmsMixIQ1234Out/20000))")]
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                self.assertEqual(report_condition(firmware.read_text(encoding="utf-8")), expected,
                                 "the every-20000 RMS report must only print with DEBUG ON")

    def test_debug_defaults_off(self):
        declaration = [t for t, _ in cpp_tokens("boolean blnDebugAdjustSN = false;")]
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                tokens = [t for t, _ in cpp_tokens(firmware.read_text(encoding="utf-8"))]
                hits = [i for i in range(len(tokens)) if tokens[i:i + len(declaration)] == declaration]
                self.assertEqual(len(hits), 1, "DEBUG must be declared once and default to OFF")


if __name__ == "__main__":
    unittest.main()
