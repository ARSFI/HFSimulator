"""LEVEL and STATUS expose the panel quantity and retain the separate max-hold."""
import unittest

from firmware_source import FIRMWARES, ROOT


def section(source, start, end):
    begin = source.index(start)
    return source[begin:source.index(end, begin)]


class SerialLevelTest(unittest.TestCase):
    def test_input_level_uses_front_panel_quantity(self):
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                source = firmware.read_text(encoding="utf-8")
                fields = section(source, "void PrintLevelFields()", "void PrintStatus()")
                self.assertIn('String(int(fltppInputForDisplay))', source)
                self.assertIn(
                    'Serial.print(" INPUT_MVPP="); Serial.print(int(fltppInputForDisplay));',
                    fields,
                )

    def test_maxhold_and_output_levels_keep_their_quantities(self):
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                source = firmware.read_text(encoding="utf-8")
                fields = section(source, "void PrintLevelFields()", "void PrintStatus()")
                for name, quantity in (
                    ("INPUT_MVPP_MAXHOLD", "fltppLPInputMeasAvg"),
                    ("LEFT_OUT_MVPP", "fltppAmpLeftOutAvg"),
                    ("RIGHT_OUT_MVPP", "fltppAmpRightOutAvg"),
                ):
                    self.assertIn(
                        f'Serial.print(" {name}="); Serial.print({quantity}, 1);', fields
                    )

    def test_level_and_status_share_the_fields(self):
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                source = firmware.read_text(encoding="utf-8")
                for start, end in (("void PrintStatus()", "void PrintLevel()"),
                                   ("void PrintLevel()", "void PrintHelp()")):
                    self.assertEqual(section(source, start, end).count("PrintLevelFields();"), 1)


if __name__ == "__main__":
    unittest.main()
