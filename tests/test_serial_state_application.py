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


def function(source: str, name: str, next_marker: str) -> str:
    start = source.index(name)
    end = source.index(next_marker, start)
    return source[start:end]


class SerialStateApplicationTest(unittest.TestCase):
    def test_bug_fixes_are_present_in_both_firmwares(self):
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                source = firmware.read_text(encoding="utf-8")
                self.assertNotIn("boolean ParseSetParameter(", source)

                sim_setter = function(
                    source,
                    "boolean ParseSetSimParameter(",
                    "boolean ParseSetBusyParameter(",
                )
                self.assertIn("SetIQTapDelays(::intMode)", sim_setter)
                self.assertIn("sine_Dnmix.frequency(7700 - intTuneOffset)", sim_setter)
                self.assertGreaterEqual(sim_setter.count("if (::intMode == 0)"), 2)

                busy_setter = function(
                    source,
                    "boolean ParseSetBusyParameter(",
                    "void UpdateTFTModeParameter(",
                )
                ch1 = function(busy_setter, "if (intMode == 7)", "if (intMode == 8)")
                self.assertRegex(ch1, r"intParam\s*<\s*9")
                self.assertNotRegex(ch1, r"intParam\s*<=\s*10")

                setup = function(source, "void setup()", "void InitializeBusy()")
                self.assertIn("fltFadeRate = 10 * fltLogs[intFadeRatePtr]", setup)
                self.assertIn(
                    "sine_VLF_Dnmix_Mod.frequency(10 * fltLogs[intFMRatePtr])",
                    setup,
                )
                self.assertIn("intFMRatePtr == 0", setup)
                self.assertIn("intFMDevPtr == 0", setup)

                dispatch = source[source.index("while  (Serial.available() > 0") :]
                self.assertIn(
                    "SimulationParameterSyntaxValid(strParameter, intSerialCmdMode) && ParseSetSimParameter",
                    dispatch,
                )
                self.assertIn(
                    "IsIntegerParameter(strParameter) && ParseSetBusyParameter",
                    dispatch,
                )

                fm_rate_display = function(
                    source,
                    "if (intMode == 10) //FM Rate",
                    "if ((intMode >= 11)",
                )
                self.assertIn("10 * fltLogs[intFMRatePtr]", fm_rate_display)
                self.assertNotIn("100 * fltLogs[intFMRatePtr]", fm_rate_display)


if __name__ == "__main__":
    unittest.main()
