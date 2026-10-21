import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PRODUCTION_FIRMWARE = ROOT / "src" / "HFSim_BFD_2_05" / "HFSim_BFD_2_05.ino"


class DisplayHeaderCleanupTest(unittest.TestCase):
    def test_non_wgn_initialization_does_not_draw_the_wgn_header(self):
        source = PRODUCTION_FIRMWARE.read_text(encoding="utf-8")
        self.assertIn("if (intMode == 0)//WGN", source)
        self.assertNotIn("if ((intMode == 0) || (! blnInitialized))//WGN", source)


if __name__ == "__main__":
    unittest.main()
