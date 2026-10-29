"""TEST3K/TEST6K are entered and left over serial by the front-panel transitions."""
import re
import unittest

from firmware_source import FIRMWARES, ROOT, cpp_tokens, delimiter_pairs


def norm(code):
    """Token text of a C++ fragment, comparable with joined token lists."""
    return " ".join(t for t, _ in cpp_tokens(code))


class Sketch:
    def __init__(self, source):
        self.source = source
        located = cpp_tokens(source)
        self.offsets = [o for _, o in located]
        self.tokens = [t for t, _ in located]
        self.pairs = delimiter_pairs([(t, 0) for t in self.tokens])
        self.text = " ".join(self.tokens)

    def body(self, name):
        """Joined body tokens of the definition of name, or None."""
        t = self.tokens
        for i in range(len(t) - 1):
            if t[i] == name and t[i + 1] == "(":
                after = self.pairs[i + 1] + 1
                if after < len(t) and t[after] == "{" and t[i - 1] in ("void", "int", "boolean", "String", "long"):
                    return " ".join(t[after + 1:self.pairs[after]])
        return None

    def block_after(self, opener):
        """Joined tokens of the {...} block following the exact source text opener."""
        start = self.source.find(opener)
        if start < 0:
            return None
        index = next(i for i, o in enumerate(self.offsets) if o >= start + len(opener))
        while self.tokens[index] != "{":
            index += 1
        return " ".join(self.tokens[index + 1:self.pairs[index]])

    def table(self, name):
        found = re.search(r"\*" + name + r"\[\]\s*=\s*\{(.*?)\};", self.source, re.DOTALL)
        return re.findall(r'"([^"]*)"', found.group(1))

    def constant(self, name):
        return int(re.search(r"\b" + name + r"\s*=\s*(\d+)\s*;", self.source).group(1))


def c_condition(expression, **names):
    """Evaluate a C boolean/arithmetic expression of plain integer names."""
    python = expression.replace("&&", " and ").replace("||", " or ")
    python = re.sub(r"!(?!=)", " not ", python)
    return eval(python, {"__builtins__": {}}, names)


def parse_sim_mode(sketch, command):
    """Model ParseSimMode() from its table, loop bound and comparison."""
    body = sketch.body("ParseSimMode")
    bound = re.search(r"for \( int i = 0 ; i < (.*?) ; i \+ \+ \)", body).group(1)
    count = c_condition(bound.replace(" ", ""), intNumModes=sketch.constant("intNumModes"))
    upper_table = "strTemp . toUpperCase ( )" in body
    for i, entry in enumerate(sketch.table("chrModes")[:count]):
        if (entry.upper() if upper_table else entry).startswith(command.upper()):
            return i
    return -1


SKETCHES = [(str(f.relative_to(ROOT)), Sketch(f.read_text(encoding="utf-8"))) for f in FIRMWARES]


class SerialTestModeTest(unittest.TestCase):
    def test_parser_resolves_test_modes(self):
        for name, sketch in SKETCHES:
          with self.subTest(firmware=name):
            for command, index in (("TEST3K", 17), ("TEST6K", 18), ("test3k", 17),
                                   ("TEST3K Sine", 17), ("TEST6K SINE", 18),
                                   ("WGN", 0), ("MPD", 4), ("BANDWIDTH", 15), ("NOPE", -1)):
                self.assertEqual(parse_sim_mode(sketch, command), index, command)

    def test_front_panel_and_serial_share_one_entry(self):
        for name, sketch in SKETCHES:
          with self.subTest(firmware=name):
            entry = sketch.body("ApplyTestMode")
            self.assertIsNotNone(entry, "no shared TEST3K/TEST6K entry function")
            self.assertTrue(entry.startswith(norm("if ((!blnSim) ||")),
                            "entry must refuse outside SIM (BUSY re-runs the mode block)")
            for flag in ("blnTestMode3K = true", "blnTestMode6K = true"):
                self.assertEqual(sketch.text.count(norm(flag)), 1, flag)
                self.assertIn(norm(flag), entry, flag)
            self.assertIn(norm("if ((intMode == 17) || (intMode == 18)) {ApplyTestMode(intMode);}"),
                          sketch.text, "mode encoder must enter through ApplyTestMode")
            serial = sketch.block_after("if ((intMode == 17) || (intMode == 18))//TEST3K, TEST6K:")
            self.assertIsNotNone(serial, "ParseSetSimParameter has no TEST3K/TEST6K setter")
            self.assertLess(serial.index("ApplyTestMode"), serial.index("SetTestFrequency"))

    def test_one_frequency_setter_owns_the_stops(self):
        for name, sketch in SKETCHES:
          with self.subTest(firmware=name):
            setter = sketch.body("SetTestFrequency")
            stops = sketch.body("TestFrequencyAtStops")
            self.assertIsNotNone(setter)
            self.assertIsNotNone(stops)
            self.assertIn(norm("intTestFreqHz = TestFrequencyAtStops(intMode, intRequestedHz);"), setter)
            for literal in ("3300", "6300"):
                self.assertIn(literal, stops.split())
            for comparison in ("intTestFreqHz >", "intTestFreqHz <"):
                self.assertNotIn(norm(comparison), sketch.text, "test stops duplicated outside the helper")
            for step in ("100", "200"):
                self.assertIn(norm(f"SetTestFrequency(intTestFreqHz + {step} * (lngENC1New - lngENC1Old));"),
                              sketch.text, "parameter encoder must use the shared setter")
            self.assertNotIn(norm("intTestFreqHz = intTestFreqHz +"), sketch.text)
            serial = sketch.block_after("if ((intMode == 17) || (intMode == 18))//TEST3K, TEST6K:")
            self.assertTrue(serial.startswith(norm(
                "if (intParam != TestFrequencyAtStops(intMode, intParam)) {return false;}")),
                "out-of-stop values are rejected before any state changes, never clamped")

    def test_test_mode_values_are_integers(self):
        for name, sketch in SKETCHES:
          with self.subTest(firmware=name):
            body = sketch.body("SimulationParameterSyntaxValid")
            condition = re.search(r"if \( (.*?) \) \{ return IsIntegerParameter", body).group(1)
            for mode in (17, 18):
                self.assertTrue(c_condition(condition.replace(" ", ""), intRequestedMode=mode))

    def test_ack_confirms_the_entered_test_mode(self):
        for name, sketch in SKETCHES:
          with self.subTest(firmware=name):
            body = sketch.body("SimulatorRequestConfirmed")
            condition = re.search(r"boolean blnModeConfirmed = (.*?) ;", body).group(1).replace(" ", "")
            for requested, live, expected in ((17, 17, True), (18, 18, True), (17, 0, False),
                                              (18, 17, False), (6, 0, True), (15, 17, True),
                                              (0, 1, False), (0, 0, True)):
                self.assertEqual(c_condition(condition, intRequestedMode=requested, intMode=live),
                                 expected, (requested, live))
            text = sketch.body("SimulatorStateParameterText")
            self.assertIn(norm("if ((intStateMode == 17) || (intStateMode == 18)) {return String(intTestFreqHz);}"), text)

    def test_status_reports_the_entered_mode_and_bandwidth(self):
        for name, sketch in SKETCHES:
          with self.subTest(firmware=name):
            entry = sketch.body("ApplyTestMode")
            self.assertIsNotNone(entry)
            self.assertIn(norm("intMode = intRequestedMode;"), entry)
            three = entry[entry.index(norm("if (intMode == 17)")):entry.index(norm("if (intMode == 18)"))]
            six = entry[entry.index(norm("if (intMode == 18)")):]
            self.assertIn(norm("intBandwidth = 3000; SetFilterBandwidth(intBandwidth);"), three)
            self.assertIn(norm("intBandwidth = 6000; SetFilterBandwidth(intBandwidth);"), six)
            status = sketch.body("PrintStatus")
            self.assertIn(norm("StateModeToken(chrModes[intMode])"), status)
            self.assertIn(norm('Serial.print(" BANDWIDTH_HZ="); Serial.print(intBandwidth);'), status)

    def test_every_exit_clears_the_test_source(self):
        for name, sketch in SKETCHES:
          with self.subTest(firmware=name):
            clear = sketch.body("ClearTestSource")
            self.assertIsNotNone(clear, "no shared exit from the test source")
            for statement in ("blnTestMode3K = false;", "blnTestMode6K = false;",
                              "blnEnableTestTone = false;", "InputTestWaveform.amplitude(0);",
                              "ampLeftOut.gain(fltLogs[intGainLevel[2]]);",
                              "ampRightOut.gain(fltLogs[intGainLevel[3]]);"):
                self.assertIn(norm(statement), clear)
            self.assertIn("ClearTestSource ( ) ;", sketch.body("ApplyChannelMode"))
            self.assertIn("ClearTestSource ( ) ;", sketch.block_after("else if (intMode < 17)"))
            busy = sketch.block_after('if (strCmd == "BUSY")')
            self.assertIn("ClearTestSource ( ) ;", busy, "serial BUSY can now follow TEST3K/TEST6K")
            self.assertLess(busy.index("ClearTestSource"), busy.index("InitializeBusy"))

    def test_help_lists_test_modes_with_the_stops(self):
        for name, sketch in SKETCHES:
          with self.subTest(firmware=name):
            line = re.search(r'F\("SIM: TEST3K:<(\d+)\.\.(\d+) Hz> \| TEST6K:<(\d+)\.\.(\d+) Hz>', sketch.source)
            self.assertIsNotNone(line, "HELP does not advertise TEST3K/TEST6K")
            stops = sketch.body("TestFrequencyAtStops")
            self.assertIsNotNone(stops)
            low = re.search(r"if \( intRequestedHz < (\d+) \)", stops).group(1)
            self.assertEqual(line.groups(), (low, "3300", low, "6300"))


if __name__ == "__main__":
    unittest.main()
