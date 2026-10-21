import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
FIRMWARES = (
    ROOT / "src" / "HFSim_BFD_2_05" / "HFSim_BFD_2_05.ino",
    ROOT
    / "hardware"
    / "Alternate hardware platforms"
    / "src"
    / "HFSim_BFD_2_05_Proto"
    / "HFSim_BFD_2_05_Proto.ino",
)

PROGRAM = (
    ("MPP:25", 3),
    ("FADE DEPTH:0", 6),
    ("FADE FREQ:0", 7),
    ("OFFSET:0", 8),
    ("CH1 IN:1", 11),
    ("CH2 IN:1", 12),
    ("CH1 OUT:1", 13),
    ("CH2 OUT:1", 14),
    ("BANDWIDTH:3000", 15),
)


class ProfileFirstProgramTest(unittest.TestCase):
    def test_nine_command_program_finishes_with_live_mpp_and_mode_3_init(self):
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                source = firmware.read_text(encoding="utf-8")
                dispatch_start = source.index(
                    "ParseSetSimParameter(strParameter, intSerialCmdMode)"
                )
                dispatch_end = source.index("//Serial Command fail", dispatch_start)
                dispatch = source[dispatch_start:dispatch_end]
                guard = re.search(
                    r"if\s*\(intSerialCmdMode\s*<\s*5\)\s*\{(?P<body>.*?)\}",
                    dispatch,
                    re.DOTALL,
                )
                self.assertIsNotNone(guard)
                self.assertIn("ApplyChannelMode(intSerialCmdMode)", guard.group("body"))

                live_mode = 0
                initialized_modes = []
                for _command, selector in PROGRAM:
                    if selector < 5:
                        live_mode = selector
                        initialized_modes.append(selector)

                self.assertEqual(live_mode, 3)
                self.assertEqual(initialized_modes, [3])

                apply_start = source.index("boolean ApplyChannelMode(")
                apply_end = source.index("int ParseSimMode", apply_start)
                apply = source[apply_start:apply_end]
                self.assertIn("intMode = intRequestedMode;", apply)
                self.assertIn("SetIQTapDelays(intMode);", apply)
                self.assertRegex(source, r"intMode\s*==\s*3.*15625")

    def test_fractional_integer_commands_are_rejected_before_mutation(self):
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                source = firmware.read_text(encoding="utf-8")
                syntax_start = source.index("boolean SimulationParameterSyntaxValid(")
                syntax_end = source.index("boolean ParseSetSimParameter(", syntax_start)
                syntax = source[syntax_start:syntax_end]
                self.assertIn("intRequestedMode <= 6", syntax)
                self.assertIn("intRequestedMode == 8", syntax)
                self.assertIn("intRequestedMode == 15", syntax)
                self.assertIn("IsIntegerParameter(strValue)", syntax)
                dispatch = source[source.index("while  (Serial.available() > 0") :]
                self.assertIn('Serial.println("?"); intSerialCmdMode = -1;', dispatch)


if __name__ == "__main__":
    unittest.main()
