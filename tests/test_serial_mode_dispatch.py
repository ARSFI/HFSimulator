import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
FIRMWARES = (
    ROOT / "src" / "HFSim_BFD_2_03" / "HFSim_BFD_2_03.ino",
    ROOT
    / "hardware"
    / "Alternate hardware platforms"
    / "src"
    / "HFSim_BFD_2_03_Proto"
    / "HFSim_BFD_2_03_Proto.ino",
)


def successful_serial_simulation_dispatch(source: str) -> str:
    start = source.index("if (ParseSetSimParameter(strParameter, intSerialCmdMode))")
    end = source.index("else { Serial.println(\"?\"); }//Serial Command fail", start)
    return source[start:end]


class SerialModeDispatchTest(unittest.TestCase):
    def test_channel_mode_commands_apply_and_reinitialize_the_selected_mode(self):
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                block = successful_serial_simulation_dispatch(
                    firmware.read_text(encoding="utf-8")
                )
                mode_branch = re.search(
                    r"if\s*\(intSerialCmdMode\s*<\s*5\)\s*\{(?P<body>.*?)\}",
                    block,
                    re.DOTALL,
                )
                self.assertIsNotNone(mode_branch, "missing channel-mode branch")
                body = mode_branch.group("body")
                self.assertRegex(body, r"\bintMode\s*=\s*intSerialCmdMode\s*;")
                self.assertRegex(body, r"\bblnInitialized\s*=\s*false\s*;")


if __name__ == "__main__":
    unittest.main()
