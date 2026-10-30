"""A successful BUSY setter's ACK confirms the state that setter wrote."""
import unittest

from firmware_source import FIRMWARES, ROOT, cpp_tokens, delimiter_pairs


def function_body(tokens, pairs, name):
    """Return the body tokens of the definition of name, or None."""
    for i, (token, _) in enumerate(tokens[:-1]):
        if token == name and tokens[i + 1][0] == "(":
            after = pairs[i + 1] + 1
            if after < len(tokens) and tokens[after][0] == "{":
                return [t for t, _ in tokens[after + 1:pairs[after]]]
    return None


def item_blocks(body, selector):
    """Map N to the tokens of each top-level `if (selector == N) {...}` block."""
    pairs = delimiter_pairs([(t, 0) for t in body])
    blocks, i = {}, 0
    while i < len(body):
        if body[i:i + 3] == ["if", "(", selector] and body[i + 3:i + 5] == ["=", "="]:
            close = pairs[i + 1]
            number = int(body[i + 5])
            if body[close + 1] == "{":
                blocks[number] = body[close + 2:pairs[close + 1]]
                i = pairs[close + 1]
        i += 1
    return blocks


def assigned(block):
    """Identifiers written by plain assignment (x = ..., x[n] = ...)."""
    names = set()
    for i, token in enumerate(block[:-1]):
        if block[i + 1] == "=" and (i + 2 >= len(block) or block[i + 2] != "="):
            j = i
            if token == "]":
                j = i - 3
            if j >= 0 and block[j] not in ("=", "!", "<", ">"):
                names.add(block[j])
    return names


def norm(code):
    """Token text of a C++ fragment, comparable with joined token lists."""
    return " ".join(t for t, _ in cpp_tokens(code))


def busy_sources(source):
    tokens = cpp_tokens(source)
    pairs = delimiter_pairs(tokens)
    return (function_body(tokens, pairs, "ParseSetBusyParameter"),
            function_body(tokens, pairs, "BusyStateParameterText"),
            function_body(tokens, pairs, "BusyRequestConfirmed"),
            [t for t, _ in tokens])


class SerialBusyAckTest(unittest.TestCase):
    def test_successful_busy_command_records_the_applied_item(self):
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                *_, flat = busy_sources(firmware.read_text(encoding="utf-8"))
                pairs = delimiter_pairs([(t, 0) for t in flat])
                call = ["ParseSetBusyParameter", "(", "strParameter", ",", "intSerialCmdMode", ")", ")"]
                starts = [i for i in range(len(flat)) if flat[i:i + len(call)] == call]
                self.assertEqual(len(starts), 1, "one serial BUSY setter dispatch")
                success = starts[0] + len(call)
                self.assertEqual(flat[success], "{")
                body = flat[success + 1:pairs[success]]
                record = ["intBusyMode", "=", "intSerialCmdMode", ";"]
                found = [i for i in range(len(body)) if body[i:i + 4] == record]
                self.assertTrue(found, "success must record intBusyMode: the ACK, TFT mirror "
                                       "and confirmation all read it")
                self.assertLess(found[0], body.index("println"),
                                "record the applied item before acknowledging")
                failure = pairs[success] + 1
                self.assertEqual(flat[failure:failure + 2], ["else", "{"])
                self.assertNotIn("intBusyMode", flat[failure + 2:pairs[failure + 1]],
                                 "a rejected command must not move the ACK item")

    def test_each_busy_item_reads_back_the_state_its_setter_writes(self):
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                setter, text, _, _ = busy_sources(firmware.read_text(encoding="utf-8"))
                setters = item_blocks(setter, "intMode")
                texts = item_blocks(text, "intStateMode")
                for item in range(10):
                    with self.subTest(item=item):
                        self.assertIn(item, setters)
                        self.assertIn(item, texts)
                        read = {t for t in texts[item] if t.isidentifier()} - {"return", "String"}
                        self.assertTrue(read, "readback must be live state, not a literal")
                        self.assertTrue(read & assigned(setters[item]),
                                        f"setter writes {sorted(assigned(setters[item]))}, "
                                        f"readback reads {sorted(read)}")

    def test_tone_setters_record_the_tone_state(self):
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                setter, *_ = busy_sources(firmware.read_text(encoding="utf-8"))
                setters = item_blocks(setter, "intMode")
                tone_on, tone_off = " ".join(setters[5]), " ".join(setters[6])
                self.assertIn(norm("intTestFreqHz = intParam ;"), tone_on)
                self.assertIn(norm("blnEnableTestTone = true ;"), tone_on)
                self.assertIn(norm("blnEnableTestTone = false ;"), tone_off)
                # Range validation is unchanged: invalid arguments still fail.
                self.assertIn(norm("intParam >= 43 ) && ( intParam <= 6300"), tone_on)
                self.assertIn(norm("if ( intParam == 0 )"), tone_off)

    def test_confirmation_still_requires_the_requested_item(self):
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                *_, confirm, _ = busy_sources(firmware.read_text(encoding="utf-8"))
                joined = " ".join(confirm)
                self.assertIn(norm("intBusyMode == intRequestedMode"), joined)
                self.assertIn(norm("BusyStateParameterText ( intRequestedMode )"), joined)


if __name__ == "__main__":
    unittest.main()
