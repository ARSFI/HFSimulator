"""Small lexical helpers for static checks of the two active C++ sketches."""
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
FIRMWARES = (
    ROOT / "src/HFSim_BFD_2_05beta/HFSim_BFD_2_05beta.ino",
    ROOT / "hardware/Alternate hardware platforms/src"
    / "HFSim_BFD_2_05beta_Proto/HFSim_BFD_2_05beta_Proto.ino",
)


def cpp_tokens(source):
    """Return (token, source offset) pairs, skipping comments and literals.

    Replace literals by an opaque token so punctuation on either side cannot
    become adjacent. Splice escaped newlines before calling this helper.
    """
    result = []
    i = 0
    while i < len(source):
        if source[i].isspace():
            i += 1
        elif source.startswith("//", i):
            end = source.find("\n", i + 2)
            i = len(source) if end < 0 else end
        elif source.startswith("/*", i):
            end = source.find("*/", i + 2)
            if end < 0:
                raise ValueError("unterminated comment")
            i = end + 2
        elif re.match(r'(?:u8|u|U|L)?R"', source[i:]):
            opening = source.index('"', i) + 1
            brace = source.find("(", opening)
            if brace < 0 or brace - opening > 16:
                raise ValueError("invalid raw string delimiter")
            closing = ")" + source[opening:brace] + '"'
            end = source.find(closing, brace + 1)
            if end < 0:
                raise ValueError("unterminated raw string")
            result.append(("<literal>", i))
            i = end + len(closing)
        elif source[i] in "\"'":
            quote, start = source[i], i
            i += 1
            while i < len(source) and source[i] != quote:
                if source[i] == "\n":
                    raise ValueError("newline in quoted literal")
                i += 2 if source[i] == "\\" else 1
            if i >= len(source):
                raise ValueError("unterminated quoted literal")
            result.append(("<literal>", start))
            i += 1
        else:
            word = re.match(r"[A-Za-z_][A-Za-z_0-9]*|[0-9]+", source[i:])
            token = word.group() if word else source[i]
            result.append((token, i))
            i += len(token)
    return result


def delimiter_pairs(tokens):
    """Match nested (), [] and {} without inspecting comments or strings."""
    pairs, stack = {}, []
    closers = {c: o for o, c in (('(', ')'), ('[', ']'), ('{', '}'))}
    for i, (token, _) in enumerate(tokens):
        if token in closers.values():
            stack.append(i)
        elif token in closers:
            if not stack or tokens[stack[-1]][0] != closers[token]:
                raise ValueError("mismatched delimiter")
            opening = stack.pop()
            pairs[opening] = i
            pairs[i] = opening
    if stack:
        raise ValueError("unclosed delimiter")
    return pairs
