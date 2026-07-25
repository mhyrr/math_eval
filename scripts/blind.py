#!/usr/bin/env python3
"""
Serve a Phase 4 problem statement WITHOUT its outcome.

    python3 scripts/blind.py --repo <formal-conjectures> --problem 89
    python3 scripts/blind.py --repo <formal-conjectures> --all
    python3 scripts/blind.py --repo <formal-conjectures> --problem 89 --raw   # STAGE 3

WHY THIS EXISTS. Phase 4 scores problems blind against a 2025-08-31 cutoff, and
sources statements from DeepMind's formal-conjectures Lean files. Those files are
pinned at 2026-07-25 and CONTAIN THE ANSWER KEY. Across the repo's 510 Erdos
files: 358 carry `@[category research solved]`, 436 carry an inline `answer(...)`
value, 113 link a machine-checked proof, and the docstrings name who resolved
what, with which model, in which month.

Reading those raw at scoring time is not a degraded blind test. It is not a blind
test. So the blinding is mechanical rather than a promise -- the same reason
memo/PREREGISTRATION.md Appendix C put the selection rule in code.

TWO MITIGATIONS, both applied:

1. EARLIEST VERSION, not HEAD. `git log --diff-filter=A` finds the commit that
   added the file; that version predates any later "solved" edit. For files that
   existed at the cutoff this is a genuinely pre-cutoff source.
2. REDACTION of what remains, applied UNIFORMLY to every problem so that the
   treatment itself carries no signal.

WHAT IS NOT FIXED, and is declared rather than defended:

- Redaction is a blacklist, and blacklists are incomplete.
- Statement-form prose ("it is not known whether X") is KEPT, because it is the
  problem statement. In a post-cutoff file it also weakly signals "still open".
  That leak runs in the dangerous direction: a scorer who senses a problem is
  still open can score it RULED_OUT and manufacture agreement with P3. The
  defense is structural rather than procedural -- none of the six axes takes
  resolution status as an input -- and the residual risk is reported in
  memo/VALIDATION.md rather than argued away.
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path

CUTOFF = "2025-08-31"

# The 56 drawn by scripts/erdos.py --sample, seed 20260725. Copied here so the
# blinding harness cannot silently serve a different set than the one committed
# in survey4/SAMPLE.md.
SAMPLE = """
1 3 7 9 11 14 32 39 41 42 61 85 89 96 100 138 142 152 153 170 184 200 212 213
218 241 244 282 288 291 295 304 312 317 330 342 455 503 539 600 617 619 653 681
789 817 830 835 847 849 853 855 868 871 881 890
""".split()

MARK = "[REDACTED]"

# Sentence-level redaction patterns, applied to PROSE ONLY (see redact()).
# Deliberately over-broad: losing statement fidelity is recoverable by searching
# pre-cutoff literature, losing blindness is not.
#
# Bare `proved` / `shown` / `established` are deliberately NOT here. "Komjáth
# [Ko13] proved that it is consistent that the answer is no" is pre-cutoff
# background, and it is exactly the input Axis C and prior_attention need.
# Killing it would trade blindness for a survey scored on nothing. What gets
# killed is resolution-of-THIS-problem language, undated recency, and any date
# after the cutoff.
OUTCOME = [
    r"\bsolved\b", r"\bresolved\b", r"\bsettled\b", r"\bsolution\b",
    r"\bdisprov\w+\b", r"\brefut\w+\b", r"\bcounterexampl\w+\b",
    r"\bformaliz\w+\b", r"\banswer(ed)?\s+(is|to this|in the)\b",
    r"\bremains? open\b", r"\bstill open\b", r"\bno longer\b",
    r"\bcategory research\b", r"\bformal_proof\b",
    # undated recency next to a resolution verb, in either order
    r"\b(recently|now|since then|as of)\b.{0,40}"
    r"\b(prov\w+|shown|solved|settled|answered|disprov\w+)\b",
    r"\b(prov\w+|shown|solved|settled|answered|disprov\w+)\b.{0,40}"
    r"\b(recently|just now|since then)\b",
    # any date at or after the cutoff year
    r"\b20(2[5-9]|[3-9]\d)\b",
]
OUTCOME_RE = re.compile("|".join(OUTCOME), re.IGNORECASE)

# Whole-line kill for bibliography entries dated at or after the cutoff. A 2026
# reference titled "A solution of Erdos problem N" is the answer key with a
# footnote marker on it; a [BoPi24] entry is literature that existed at scoring
# time and is legitimately available. Stage 3's CATCH-UP / NEW WORK split reads
# exactly this block, which is why it cannot be read at stage 2.
REF_KILL_RE = re.compile(
    r"\b20(2[5-9]|[3-9]\d)\b"              # a four-digit year at/after the cutoff
    r"|\[[A-Za-z\-'’]+2[5-9]\]"            # citation key [Xx25]..[Xx29]. NOT [3-9]\d,
                                           # which reads [ErHa66] as a 2066 paper.
    r"|arXiv:\s*(25(09|1[0-2])|2[6-9]\d\d)\."   # arXiv YYMM id after 2025-08
)

# Who might have done it. CASE-SENSITIVE and matched separately, because
# case-insensitive `Aleph` (the prover) eats `aleph` (the cardinal) and takes
# the theorem statement with it. Found by testing the harness on #1067, which is
# on the exclusion list.
AGENT_RE = re.compile(
    r"\b(GPT|ChatGPT|Gemini|Claude|Aristotle|AlphaProof|AlphaEvolve|"
    r"Aleph Prover|DeepSeek|Grok|Llama|Qwen|LLM|AI)\b")

# Prose lives in block comments (`/- -/`, `/-- -/`, `/-! -/`) and line comments.
# Lean code carries no outcome text except `answer(...)` and the category
# attribute, both handled structurally, so code is left intact.
COMMENT_RE = re.compile(r"/-.*?-/|--[^\n]*", re.S)

LICENSE_RE = re.compile(r"^/-\s*\nCopyright.*?^-/\s*\n", re.S | re.M)
# `answer(...)`, balanced to one nesting level -- enough for every occurrence in
# this repo, and the fallback below catches anything deeper.
ANSWER_RE = re.compile(r"answer\(([^()]|\([^()]*\))*\)")
CATEGORY_RE = re.compile(r"@\[\s*category\b[^\]]*\]", re.S)


def sh(cwd, *args):
    r = subprocess.run(args, cwd=cwd, capture_output=True, text=True)
    if r.returncode:
        sys.exit(f"{' '.join(args)} failed:\n{r.stderr}")
    return r.stdout


def earliest(repo, rel):
    """Content and date of the commit that ADDED this file."""
    # No --follow: rename-following walks off this path onto unrelated files in
    # a blobless clone and lands on the repo's initial commit.
    log = sh(repo, "git", "log", "--diff-filter=A",
             "--format=%H %cI", "--", rel).strip().splitlines()
    if not log:
        return None, None
    sha, date = log[-1].split()
    return sh(repo, "git", "show", f"{sha}:{rel}"), date


def _prose(seg, counter):
    """Sentence-level redaction inside one comment segment."""
    out = []
    for line in seg.splitlines():
        # bibliography entries die whole -- author and title carry the outcome
        if re.match(r"\s*-\s*\[", line) and REF_KILL_RE.search(line):
            out.append(MARK)
            counter[0] += 1
            continue
        # split on sentence ends so one bad sentence does not take a whole
        # paragraph of statement with it
        keep = []
        for s in re.split(r"(?<=[.!?])\s+", line):
            if OUTCOME_RE.search(s) or AGENT_RE.search(s):
                keep.append(MARK)
                counter[0] += 1
            else:
                keep.append(s)
        out.append(" ".join(keep))
    return "\n".join(out)


def redact(text):
    counter = [0]
    text = LICENSE_RE.sub("", text)

    text, k = CATEGORY_RE.subn("@[category " + MARK + "]", text)
    counter[0] += k
    # placeholder first, so the leftover check below cannot re-match our own
    # substitution -- `answer([REDACTED])` still contains the string `answer(`
    text, k = ANSWER_RE.subn("answer<" + MARK + ">", text)
    counter[0] += k
    if "answer(" in text:                      # deeper nesting than the pattern
        text = text.replace("answer(", "answer<" + MARK + ">(")
        counter[0] += 1

    text = COMMENT_RE.sub(lambda m: _prose(m.group(0), counter), text)
    return text, counter[0]


def serve(repo, num, raw=False):
    rel = f"FormalConjectures/ErdosProblems/{num}.lean"
    if raw:
        body = (Path(repo) / rel).read_text()
        print(f"===== #{num} RAW (HEAD) -- CONTAINS OUTCOMES =====\n{body}")
        return

    body, date = earliest(repo, rel)
    if body is None:
        print(f"===== #{num} -- NO LEAN FILE =====\n")
        return
    body, n = redact(body)
    era = "PRE-CUTOFF" if date[:10] <= CUTOFF else "post-cutoff file version"
    print(f"===== #{num} =====")
    print(f"source: earliest version, added {date[:10]} ({era}); "
          f"{n} redactions applied")
    print(body.strip() + "\n")


# Anything matching these in redacted output is a blindness failure.
LEAK = re.compile(r"category research (solved|open)"
                  r"|answer\((?!<)"          # a surviving answer payload
                  r"|formal_proof using")


def selftest(repo):
    """Redact every Erdos file NOT in the sample and assert no leak survives.

    Deliberately skips the 56 sampled problems: a self-test that printed which
    of THOSE leaked would be the leak. 454 files is plenty to validate the
    rules, and they are the files this session is allowed to look at.
    """
    d = Path(repo) / "FormalConjectures/ErdosProblems"
    files = [f for f in sorted(d.glob("*.lean")) if f.stem not in set(SAMPLE)]
    bad = []
    for f in files:
        body, _ = earliest(repo, f"FormalConjectures/ErdosProblems/{f.name}")
        if body is None:
            continue
        red, _ = redact(body)
        for m in LEAK.finditer(red):
            bad.append((f.stem, m.group(0)))
    print(f"checked {len(files)} non-sampled files "
          f"({len(SAMPLE)} sampled files skipped by design)")
    if bad:
        print(f"FAIL -- {len(bad)} leaks survived redaction:")
        for stem, hit in bad[:40]:
            print(f"  #{stem}: {hit!r}")
        sys.exit(1)
    print("PASS -- no status marker, answer payload or proof link survived")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", required=True, help="formal-conjectures checkout")
    ap.add_argument("--selftest", action="store_true",
                    help="verify redaction over every non-sampled Erdos file")
    ap.add_argument("--problem", nargs="+", metavar="N")
    ap.add_argument("--all", action="store_true", help="serve all 56 sampled")
    ap.add_argument("--raw", action="store_true",
                    help="UNREDACTED HEAD file. Stage 3 only -- reveals outcomes.")
    a = ap.parse_args()

    if a.selftest:
        return selftest(a.repo)

    nums = SAMPLE if a.all else (a.problem or [])
    if not nums:
        return ap.print_help()
    if a.raw and a.all:
        sys.exit("refusing --raw --all")
    for n in nums:
        serve(a.repo, n, a.raw)


if __name__ == "__main__":
    main()
