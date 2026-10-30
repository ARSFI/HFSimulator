"""Reject accidental empty control bodies and duplicate statements in firmware."""
import unittest

from firmware_source import FIRMWARES, ROOT, cpp_tokens, delimiter_pairs


def empty_statements(source):
    tokens = cpp_tokens(source)
    pairs = delimiter_pairs(tokens)
    words = [token for token, _ in tokens]

    def statement_end(start):
        """Find the token after a do body, including nested control statements."""
        token = words[start]
        if token == "{":
            return pairs[start] + 1
        if token == "do":
            trailer = statement_end(start + 1)
            if words[trailer:trailer + 2] != ["while", "("]:
                raise ValueError("missing do-while trailer")
            end = pairs[trailer + 1] + 1
            if words[end] != ";":
                raise ValueError("missing do-while terminator")
            return end + 1
        if token in ("if", "for", "while", "switch"):
            end = statement_end(pairs[start + 1] + 1)
            if token == "if" and words[end:end + 1] == ["else"]:
                end = statement_end(end + 1)
            return end
        i = start
        while i < len(words):
            if words[i] == ";":
                return i + 1
            i = pairs[i] + 1 if words[i] in ("(", "[", "{") else i + 1
        raise ValueError("unterminated statement")

    trailers = set()
    for_headers = set()
    for i, word in enumerate(words):
        if word == "do":
            trailer = statement_end(i + 1)
            if words[trailer:trailer + 2] != ["while", "("]:
                raise ValueError("missing do-while trailer")
            trailers.add(trailer)
        if word == "for" and words[i + 1:i + 2] == ["("]:
            for_headers.update(range(i + 2, pairs[i + 1]))

    findings = []
    for i, word in enumerate(words):
        if word in ("if", "for", "while") and i not in trailers:
            if words[i + 1:i + 2] == ["("]:
                body = pairs[i + 1] + 1
                if words[body:body + 1] == [";"]:
                    findings.append((tokens[i][1], "empty " + word + " body"))
        if word == ";" and words[i + 1:i + 2] == [";"]:
            if i not in for_headers or i + 1 not in for_headers:
                findings.append((tokens[i + 1][1], "doubled semicolon"))
    return [f"line {source.count(chr(10), 0, offset) + 1}: {message}"
            for offset, message in findings]


class EmptyStatementsTest(unittest.TestCase):
    def test_firmware_has_no_accidental_empty_statements(self):
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                self.assertEqual(empty_statements(firmware.read_text(encoding="utf-8")), [])

    def test_test_modes_still_force_bandwidth(self):
        for firmware in FIRMWARES:
            with self.subTest(firmware=str(firmware.relative_to(ROOT))):
                words = [t for t, _ in cpp_tokens(firmware.read_text(encoding="utf-8"))]
                for mode, bandwidth in ((17, 3000), (18, 6000)):
                    marker = [t for t, _ in cpp_tokens(f"if (intMode == {mode})")]
                    starts = [i for i in range(len(words))
                              if words[i:i + len(marker)] == marker]
                    pairs = delimiter_pairs([(w, i) for i, w in enumerate(words)])
                    bodies = []
                    for match in starts:
                        start = match + len(marker)
                        if words[start] == "{":
                            body = words[start + 1:pairs[start]]
                            if "blnTestMode3K" in body and "SetFilterBandwidth" in body:
                                bodies.append(body)
                    self.assertEqual(len(bodies), 1)
                    body = bodies[0]
                    assignment = [t for t, _ in cpp_tokens(
                        f"intBandwidth = {bandwidth}; SetFilterBandwidth(intBandwidth);")]
                    index = body.index("intBandwidth")
                    self.assertEqual(body[index:index + len(assignment)], assignment)


class EmptyStatementScannerTest(unittest.TestCase):
    def test_empty_control_bodies_with_nested_parentheses(self):
        for keyword in ("if", "for", "while"):
            with self.subTest(keyword=keyword):
                source = f"{keyword} (fn((x), other(y))) /* comment */ ; {{ work(); }}"
                self.assertEqual(empty_statements(source), [f"line 1: empty {keyword} body"])

    def test_duplicate_semicolons_with_whitespace_and_comments(self):
        for gap in ("", " ", "\n", "/* ; if(x); */", "// text\n"):
            with self.subTest(gap=gap):
                self.assertEqual(len(empty_statements("work();" + gap + ";")), 1)

    def test_for_headers_and_do_while_trailers_are_not_empty_bodies(self):
        for source in (
            "for (;;) { work(); }",
            "for (i = fn(a, (b)); ; ++i) work();",
            "do { work(); } while (fn((x)));",
            "do work(); while (ready());",
            "do do { work(); } while (inner()); while (outer());",
            "do if (x) work(); else other(); while (ready());",
            "do while (ready()) { work(); } while (again());",
        ):
            with self.subTest(source=source):
                self.assertEqual(empty_statements(source), [])
        self.assertEqual(len(empty_statements("for (;;);")), 1)
        self.assertEqual(len(empty_statements("do {} while (x);;")), 1)
        self.assertEqual(len(empty_statements("do { while (x); } while (y);")), 1)

    def test_comments_and_literals_hide_control_tokens_and_delimiters(self):
        source = r'''
            // if (x); ;; { (
            /* while (x); ;; } ) */
            const char *s = "escaped \" if (x); ;; {";
            const char c = '\'';
            const char *raw = u8R"tag(while (x); ;; " ) } )tag";
            if (fn("not a )")) { work(); }
        '''
        self.assertEqual(empty_statements(source), [])
        self.assertEqual(len(empty_statements('if (fn(")"));')), 1)
        self.assertEqual(len(empty_statements('work(); /* ignored */ ;')), 1)

    def test_incomplete_or_mismatched_input_is_rejected(self):
        for source in ("if ((x);", "if (x]) work();", "/* open", '"open',
                       'R"tag(open', "do work();", "if ('x\n') {}"):
            with self.subTest(source=source):
                with self.assertRaises((ValueError, IndexError)):
                    empty_statements(source)


if __name__ == "__main__":
    unittest.main()
