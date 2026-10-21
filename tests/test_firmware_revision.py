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


class FirmwareRevisionTest(unittest.TestCase):
    def test_sketches_follow_the_2_05_naming_convention(self):
        self.assertTrue(PRODUCTION.is_file())
        self.assertTrue(PROTO.is_file())
        # Previous sketch directories must not linger after the rename.
        self.assertFalse((ROOT / "src" / "HFSim_BFD_2_03").exists())
        self.assertFalse((ROOT / "src" / "HFSim_BFD_2_04").exists())
        proto_src = ROOT / "hardware" / "Alternate hardware platforms" / "src"
        self.assertFalse((proto_src / "HFSim_BFD_2_03_Proto").exists())
        self.assertFalse((proto_src / "HFSim_BFD_2_04_Proto").exists())

    def test_live_revision_surfaces_report_2_05(self):
        for firmware in (PRODUCTION, PROTO):
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                source = firmware.read_text(encoding="utf-8")
                self.assertIn('#define HFSIM_FIRMWARE_REVISION "2.05"', source)
                self.assertIn('Serial.print(" REV=" HFSIM_FIRMWARE_REVISION)', source)
                self.assertIn('F("HELP IONOS SIM Rev " HFSIM_FIRMWARE_REVISION)', source)
                # Revision surfaces are macro-driven, never hardcoded old literals.
                self.assertNotIn('strRevision = "    Rev 2.03', source)
                self.assertNotIn('strRevision = "    Rev 2.04', source)


if __name__ == "__main__":
    unittest.main()
