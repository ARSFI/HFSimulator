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


def successful_serial_simulation_dispatch(source: str) -> str:
    start = source.index("ParseSetSimParameter(strParameter, intSerialCmdMode)")
    end = source.index("//Serial Command fail", start)
    return source[start:end]


class SerialModeDispatchTest(unittest.TestCase):
    def test_channel_mode_commands_apply_and_reinitialize_the_selected_mode(self):
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                source = firmware.read_text(encoding="utf-8")
                block = successful_serial_simulation_dispatch(source)
                guard = re.search(
                    r"if\s*\(intSerialCmdMode\s*<\s*5\)\s*\{(?P<body>.*?)\}",
                    block,
                    re.DOTALL,
                )
                self.assertIsNotNone(
                    guard,
                    "only channel-profile selectors may replace the live mode",
                )
                self.assertIn("ApplyChannelMode(intSerialCmdMode)", guard.group("body"))
                self.assertLess(
                    guard.group("body").index("ApplyChannelMode(intSerialCmdMode)"),
                    guard.group("body").index('Serial.println("OK")'),
                    "the DSP profile must be applied before its acknowledgment",
                )

                apply = source[
                    source.index("boolean ApplyChannelMode(") : source.index(
                        "int ParseSimMode", source.index("boolean ApplyChannelMode(")
                    )
                ]
                self.assertIn("intMode = intRequestedMode;", apply)
                self.assertIn("SetIQTapDelays(intMode);", apply)
                self.assertIn("ulngAppliedGeneration++;", apply)


if __name__ == "__main__":
    unittest.main()
