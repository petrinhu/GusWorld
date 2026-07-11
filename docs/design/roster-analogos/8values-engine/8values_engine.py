#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
#
# 8values Engine (Python port) -- GusWorld design tool
# ======================================================
#
# THIS FILE IS A DERIVATIVE WORK (fork/port) OF "8values":
#   Original project: https://github.com/8values/8values.github.io
#   Original site:     http://8values.github.io
#   Original copyright: Copyright (c) 2020 8values. http://8values.github.io.
#   Original license:   MIT License (full text in LICENSE_8VALUES_ORIGINAL.txt
#                        and in ATTRIBUTION.md, same directory as this file)
#
# The 70 quiz questions, their per-axis weights, the scoring/normalization
# formula, the axis-label thresholds, and the 52-ideology matching table
# below are TRANSCRIBED VERBATIM from the original 8values repository
# (questions.js, ideologies.js, quiz.html, results.html @ commit fetched
# 2026-07-11 from the `master` branch). Full attribution and license terms
# in ATTRIBUTION.md.
#
# WHAT GUSWORLD ADDED (not present in the original):
#   - This Python port itself (a faithful, standalone re-implementation of
#     the JS scoring engine, usable offline / from the command line / from
#     other scripts, no browser needed).
#   - A single scalar "rightness" (0.0-1.0) derived from the Economic axis
#     (Markets = 1.0 = right, Equality = 0.0 = left), per GusWorld design
#     lead's explicit rule, plus the Societal (Progress<->Tradition) axis
#     exposed separately as a manual tie-breaker field.
#   - JSON/stdin input plumbing and a human-readable CLI report.
#   - PERGUNTAS.md: the 70 questions numbered for persona-agents to answer.
#
# Purpose in GusWorld: classify the political spectrum of the 21-figure
# historical roster (docs/design/roster-analogos/) for narrative/design
# consistency. This is a DESIGN TOOL, not shipped game code.
#
# License of this file: MIT (see LICENSE_8VALUES_ORIGINAL.txt for the
# original notice, which is preserved verbatim as required by the MIT
# license of the upstream work; this derivative is offered under the same
# MIT terms).

"""8values Engine -- Python port of the 8values political-spectrum quiz.

Ports the scoring formula from https://github.com/8values/8values.github.io
(quiz.html + results.html + questions.js + ideologies.js) into a standalone,
offline, callable Python module/CLI.

INPUT FORMAT
------------
Exactly 70 answers, in the SAME ORDER as PERGUNTAS.md (questions 1-70),
each answer one of the 5 tokens (case-insensitive):

    "sa"  -> Strongly Agree     (+1.0 multiplier)
    "a"   -> Agree              (+0.5 multiplier)
    "n"   -> Neutral/Unsure     ( 0.0 multiplier)
    "d"   -> Disagree           (-0.5 multiplier)
    "sd"  -> Strongly Disagree  (-1.0 multiplier)

Provide the 70 answers as a JSON array of strings, e.g.:

    ["sa", "a", "n", ..., "sd"]   (exactly 70 elements)

or as a JSON object with an "answers" key:

    {"answers": ["sa", "a", "n", ..., "sd"]}

Two ways to feed it in:

    (a) File argument:
        python3 8values_engine.py answers.json

    (b) stdin:
        cat answers.json | python3 8values_engine.py
        echo '["sa","a", ...]' | python3 8values_engine.py

OUTPUT
------
Prints a human-readable report (4 axes in %, closest ideology label,
rightness scalar) to stdout, and ALSO prints a machine-parseable line:

    RIGHTNESS=0.NNN

so callers can `grep RIGHTNESS= output.txt | cut -d= -f2`.

SCORING FORMULA (confirmed verbatim from quiz.html of the original repo)
--------------------------------------------------------------------------
For each of the 4 axes (econ, dipl, govt, scty):

    max_axis   = sum(abs(question.effect[axis]) for question in QUESTIONS)
    score_axis = sum(multiplier(answer_i) * question_i.effect[axis]
                      for i in 0..69)
    pct_axis   = round(100 * (max_axis + score_axis) / (2 * max_axis), 1)

`pct_axis` is the percentage on the "positive" pole of that axis:
    econ -> Equality %   (Markets % = 100 - Equality %)
    dipl -> Peace %       (Nation/Might % = 100 - Peace %)
    govt -> Liberty %     (Authority % = 100 - Liberty %)
    scty -> Progress %    (Tradition % = 100 - Progress %)

IDEOLOGY MATCHING (confirmed verbatim from results.html)
----------------------------------------------------------
Closest ideology = argmin over the 52-entry IDEOLOGIES table of:

    dist = (ideology.econ - equality_pct) ** 2
         + (ideology.govt - liberty_pct) ** 2
         + abs(ideology.dipl - peace_pct) ** 1.73856063
         + abs(ideology.scty - progress_pct) ** 1.73856063

(note the asymmetric exponents: econ/govt use a plain square; dipl/scty use
the exponent 1.73856063, exactly as in the original JS source.)

RIGHTNESS SCALAR (GusWorld-specific rule, NOT part of the original 8values)
------------------------------------------------------------------------------
Per the GusWorld design lead's explicit rule:

    rightness = economic_markets_pct / 100.0

    i.e. Markets (economic right) = 1.0, Equality (economic left) = 0.0.
    economic_markets_pct = 100 - economic_equality_pct.

The Societal axis (Progress <-> Tradition) is reported as a SEPARATE field
(`societal_tradition_pct`, where Tradition = more right-coded) for MANUAL
desemate/tie-breaking by a human -- it does NOT feed into the `rightness`
scalar itself.
"""

from __future__ import annotations

import json
import sys
from dataclasses import dataclass, field
from typing import Iterable, Sequence


ANSWER_MULTIPLIERS: dict[str, float] = {
    "sa": 1.0,
    "a": 0.5,
    "n": 0.0,
    "d": -0.5,
    "sd": -1.0,
}

AXES: tuple[str, str, str, str] = ("econ", "dipl", "govt", "scty")

# Axis label thresholds, ported verbatim from results.html's setLabel().
# Applied to the axis's "positive pole" percentage (equality/peace/liberty/progress).
_LABEL_THRESHOLDS: tuple[tuple[float, int], ...] = (
    (90, 0), (75, 1), (60, 2), (40, 3), (25, 4), (10, 5), (0, 6),
)

ECON_LABELS = ["Communist", "Socialist", "Social", "Centrist", "Market", "Capitalist", "Laissez-Faire"]
DIPL_LABELS = ["Cosmopolitan", "Internationalist", "Peaceful", "Balanced", "Patriotic", "Nationalist", "Chauvinist"]
GOVT_LABELS = ["Anarchist", "Libertarian", "Liberal", "Moderate", "Statist", "Authoritarian", "Totalitarian"]
SCTY_LABELS = ["Revolutionary", "Very Progressive", "Progressive", "Neutral", "Traditional", "Very Traditional", "Reactionary"]


def _axis_label(value: float, labels: Sequence[str]) -> str:
    """Port of setLabel() in results.html. `value` is the axis's positive-pole %."""
    if value > 100:
        return ""
    for threshold, idx in _LABEL_THRESHOLDS:
        # Mirrors the JS chain: >90 -> [0], >75 -> [1], >60 -> [2],
        # >=40 -> [3], >=25 -> [4], >=10 -> [5], >=0 -> [6]
        if idx <= 2:
            if value > threshold:
                return labels[idx]
        else:
            if value >= threshold:
                return labels[idx]
    return ""


# ---------------------------------------------------------------------------
# QUESTIONS: verbatim from questions.js (70 entries). Order is authoritative
# and MUST match PERGUNTAS.md numbering 1-70.
# ---------------------------------------------------------------------------
QUESTIONS: list[dict] = \
[
    {
        "question": "Oppression by corporations is more of a concern than oppression by governments.",
        "effect": {
            "econ": 10,
            "dipl": 0,
            "govt": -5,
            "scty": 0
        }
    },
    {
        "question": "It is necessary for the government to intervene in the economy to protect consumers.",
        "effect": {
            "econ": 10,
            "dipl": 0,
            "govt": 0,
            "scty": 0
        }
    },
    {
        "question": "The freer the markets, the freer the people.",
        "effect": {
            "econ": -10,
            "dipl": 0,
            "govt": 0,
            "scty": 0
        }
    },
    {
        "question": "It is better to maintain a balanced budget than to ensure welfare for all citizens.",
        "effect": {
            "econ": -10,
            "dipl": 0,
            "govt": 0,
            "scty": 0
        }
    },
    {
        "question": "Publicly-funded research is more beneficial to the people than leaving it to the market.",
        "effect": {
            "econ": 10,
            "dipl": 0,
            "govt": 0,
            "scty": 10
        }
    },
    {
        "question": "Tariffs on international trade are important to encourage local production.",
        "effect": {
            "econ": 5,
            "dipl": 0,
            "govt": -10,
            "scty": 0
        }
    },
    {
        "question": "From each according to his ability, to each according to his needs.",
        "effect": {
            "econ": 10,
            "dipl": 0,
            "govt": 0,
            "scty": 0
        }
    },
    {
        "question": "It would be best if social programs were abolished in favor of private charity.",
        "effect": {
            "econ": -10,
            "dipl": 0,
            "govt": 0,
            "scty": 0
        }
    },
    {
        "question": "Taxes should be increased on the rich to provide for the poor.",
        "effect": {
            "econ": 10,
            "dipl": 0,
            "govt": 0,
            "scty": 0
        }
    },
    {
        "question": "Inheritance is a legitimate form of wealth.",
        "effect": {
            "econ": -10,
            "dipl": 0,
            "govt": 0,
            "scty": -5
        }
    },
    {
        "question": "Basic utilities like roads and electricity should be publicly owned.",
        "effect": {
            "econ": 10,
            "dipl": 0,
            "govt": 0,
            "scty": 0
        }
    },
    {
        "question": "Government intervention is a threat to the economy.",
        "effect": {
            "econ": -10,
            "dipl": 0,
            "govt": 0,
            "scty": 0
        }
    },
    {
        "question": "Those with a greater ability to pay should receive better healthcare.",
        "effect": {
            "econ": -10,
            "dipl": 0,
            "govt": 0,
            "scty": 0
        }
    },
    {
        "question": "Quality education is a right of all people.",
        "effect": {
            "econ": 10,
            "dipl": 0,
            "govt": 0,
            "scty": 5
        }
    },
    {
        "question": "The means of production should belong to the workers who use them.",
        "effect": {
            "econ": 10,
            "dipl": 0,
            "govt": 0,
            "scty": 0
        }
    },
    {
        "question": "The United Nations should be abolished.",
        "effect": {
            "econ": 0,
            "dipl": -10,
            "govt": -5,
            "scty": 0
        }
    },
    {
        "question": "Military action by our nation is often necessary to protect it.",
        "effect": {
            "econ": 0,
            "dipl": -10,
            "govt": -10,
            "scty": 0
        }
    },
    {
        "question": "I support regional unions, such as the European Union.",
        "effect": {
            "econ": -5,
            "dipl": 10,
            "govt": 10,
            "scty": 5
        }
    },
    {
        "question": "It is important to maintain our national sovereignty.",
        "effect": {
            "econ": 0,
            "dipl": -10,
            "govt": -5,
            "scty": 0
        }
    },
    {
        "question": "A united world government would be beneficial to mankind.",
        "effect": {
            "econ": 0,
            "dipl": 10,
            "govt": 0,
            "scty": 0
        }
    },
    {
        "question": "It is more important to retain peaceful relations than to further our strength.",
        "effect": {
            "econ": 0,
            "dipl": 10,
            "govt": 0,
            "scty": 0
        }
    },
    {
        "question": "Wars do not need to be justified to other countries.",
        "effect": {
            "econ": 0,
            "dipl": -10,
            "govt": -10,
            "scty": 0
        }
    },
    {
        "question": "Military spending is a waste of money.",
        "effect": {
            "econ": 0,
            "dipl": 10,
            "govt": 10,
            "scty": 0
        }
    },
    {
        "question": "International aid is a waste of money.",
        "effect": {
            "econ": -5,
            "dipl": -10,
            "govt": 0,
            "scty": 0
        }
    },
    {
        "question": "My nation is great.",
        "effect": {
            "econ": 0,
            "dipl": -10,
            "govt": 0,
            "scty": 0
        }
    },
    {
        "question": "Research should be conducted on an international scale.",
        "effect": {
            "econ": 0,
            "dipl": 10,
            "govt": 0,
            "scty": 10
        }
    },
    {
        "question": "Governments should be accountable to the international community.",
        "effect": {
            "econ": 0,
            "dipl": 10,
            "govt": 5,
            "scty": 0
        }
    },
    {
        "question": "Even when protesting an authoritarian government, violence is not acceptable.",
        "effect": {
            "econ": 0,
            "dipl": 5,
            "govt": -5,
            "scty": 0
        }
    },
    {
        "question": "My religious values should be spread as much as possible.",
        "effect": {
            "econ": 0,
            "dipl": -5,
            "govt": -10,
            "scty": -10
        }
    },
    {
        "question": "Our nation's values should be spread as much as possible.",
        "effect": {
            "econ": 0,
            "dipl": -10,
            "govt": -5,
            "scty": 0
        }
    },
    {
        "question": "It is very important to maintain law and order.",
        "effect": {
            "econ": 0,
            "dipl": -5,
            "govt": -10,
            "scty": -5
        }
    },
    {
        "question": "The general populace makes poor decisions.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": -10,
            "scty": 0
        }
    },
    {
        "question": "Physician-assisted suicide should be legal.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": 10,
            "scty": 0
        }
    },
    {
        "question": "The sacrifice of some civil liberties is necessary to protect us from acts of terrorism.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": -10,
            "scty": 0
        }
    },
    {
        "question": "Government surveillance is necessary in the modern world.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": -10,
            "scty": 0
        }
    },
    {
        "question": "The very existence of the state is a threat to our liberty.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": 10,
            "scty": 0
        }
    },
    {
        "question": "Regardless of political opinions, it is important to side with your country.",
        "effect": {
            "econ": 0,
            "dipl": -10,
            "govt": -10,
            "scty": -5
        }
    },
    {
        "question": "All authority should be questioned.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": 10,
            "scty": 5
        }
    },
    {
        "question": "A hierarchical state is best.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": -10,
            "scty": 0
        }
    },
    {
        "question": "It is important that the government follows the majority opinion, even if it is wrong.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": 10,
            "scty": 0
        }
    },
    {
        "question": "The stronger the leadership, the better.",
        "effect": {
            "econ": 0,
            "dipl": -10,
            "govt": -10,
            "scty": 0
        }
    },
    {
        "question": "Democracy is more than a decision-making process.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": 10,
            "scty": 0
        }
    },
    {
        "question": "Environmental regulations are essential.",
        "effect": {
            "econ": 5,
            "dipl": 0,
            "govt": 0,
            "scty": 10
        }
    },
    {
        "question": "A better world will come from automation, science, and technology.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": 0,
            "scty": 10
        }
    },
    {
        "question": "Children should be educated in religious or traditional values.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": -5,
            "scty": -10
        }
    },
    {
        "question": "Traditions are of no value on their own.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": 0,
            "scty": 10
        }
    },
    {
        "question": "Religion should play a role in government.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": -10,
            "scty": -10
        }
    },
    {
        "question": "Churches should be taxed the same way other institutions are taxed.",
        "effect": {
            "econ": 5,
            "dipl": 0,
            "govt": 0,
            "scty": 10
        }
    },
    {
        "question": "Climate change is currently one of the greatest threats to our way of life.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": 0,
            "scty": 10
        }
    },
    {
        "question": "It is important that we work as a united world to combat climate change.",
        "effect": {
            "econ": 0,
            "dipl": 10,
            "govt": 0,
            "scty": 10
        }
    },
    {
        "question": "Society was better many years ago than it is now.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": 0,
            "scty": -10
        }
    },
    {
        "question": "It is important that we maintain the traditions of our past.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": 0,
            "scty": -10
        }
    },
    {
        "question": "It is important that we think in the long term, beyond our lifespans.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": 0,
            "scty": 10
        }
    },
    {
        "question": "Reason is more important than maintaining our culture.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": 0,
            "scty": 10
        }
    },
    {
        "question": "Drug use should be legalized or decriminalized.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": 10,
            "scty": 2
        }
    },
    {
        "question": "Same-sex marriage should be legal.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": 10,
            "scty": 10
        }
    },
    {
        "question": "No cultures are superior to others.",
        "effect": {
            "econ": 0,
            "dipl": 10,
            "govt": 5,
            "scty": 10
        }
    },
    {
        "question": "Sex outside marriage is immoral.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": -5,
            "scty": -10
        }
    },
    {
        "question": "If we accept migrants at all, it is important that they assimilate into our culture.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": -5,
            "scty": -10
        }
    },
    {
        "question": "Abortion should be prohibited in most or all cases.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": -10,
            "scty": -10
        }
    },
    {
        "question": "Gun ownership should be prohibited for those without a valid reason.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": -10,
            "scty": 0
        }
    },
    {
        "question": "I support single-payer, universal healthcare.",
        "effect": {
            "econ": 10,
            "dipl": 0,
            "govt": 0,
            "scty": 0
        }
    },
    {
        "question": "Prostitution should be illegal.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": -10,
            "scty": -10
        }
    },
    {
        "question": "Maintaining family values is essential.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": 0,
            "scty": -10
        }
    },
    {
        "question": "To chase progress at all costs is dangerous.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": 0,
            "scty": -10
        }
    },
    {
        "question": "Genetic modification is a force for good, even on humans.",
        "effect": {
            "econ": 0,
            "dipl": 0,
            "govt": 0,
            "scty": 10
        }
    },
    {
        "question": "We should open our borders to immigration.",
        "effect": {
            "econ": 0,
            "dipl": 10,
            "govt": 10,
            "scty": 0
        }
    },
    {
        "question": "Governments should be as concerned about foreigners as they are about their own citizens.",
        "effect": {
            "econ": 0,
            "dipl": 10,
            "govt": 0,
            "scty": 0
        }
    },
    {
        "question": "All people - regardless of factors like culture or sexuality - should be treated equally.",
        "effect": {
            "econ": 10,
            "dipl": 10,
            "govt": 10,
            "scty": 10
        }
    },
    {
        "question": "It is important that we further my group's goals above all others.",
        "effect": {
            "econ": -10,
            "dipl": -10,
            "govt": -10,
            "scty": -10
        }
    }
]

assert len(QUESTIONS) == 70, f"expected 70 questions, got {len(QUESTIONS)}"

# ---------------------------------------------------------------------------
# IDEOLOGIES: verbatim from ideologies.js (52 entries).
# ---------------------------------------------------------------------------
IDEOLOGIES: list[dict] = \
[
    {
        "name": "Anarcho-Communism",
        "stats": {
            "econ": 100,
            "dipl": 50,
            "govt": 100,
            "scty": 90
        }
    },
    {
        "name": "Libertarian Communism",
        "stats": {
            "econ": 100,
            "dipl": 70,
            "govt": 80,
            "scty": 80
        }
    },
    {
        "name": "Trotskyism",
        "stats": {
            "econ": 100,
            "dipl": 100,
            "govt": 60,
            "scty": 80
        }
    },
    {
        "name": "Marxism",
        "stats": {
            "econ": 100,
            "dipl": 70,
            "govt": 40,
            "scty": 80
        }
    },
    {
        "name": "De Leonism",
        "stats": {
            "econ": 100,
            "dipl": 30,
            "govt": 30,
            "scty": 80
        }
    },
    {
        "name": "Leninism",
        "stats": {
            "econ": 100,
            "dipl": 40,
            "govt": 20,
            "scty": 70
        }
    },
    {
        "name": "Stalinism/Maoism",
        "stats": {
            "econ": 100,
            "dipl": 20,
            "govt": 0,
            "scty": 60
        }
    },
    {
        "name": "Religious Communism",
        "stats": {
            "econ": 100,
            "dipl": 50,
            "govt": 30,
            "scty": 30
        }
    },
    {
        "name": "State Socialism",
        "stats": {
            "econ": 80,
            "dipl": 30,
            "govt": 30,
            "scty": 70
        }
    },
    {
        "name": "Theocratic Socialism",
        "stats": {
            "econ": 80,
            "dipl": 50,
            "govt": 30,
            "scty": 20
        }
    },
    {
        "name": "Religious Socialism",
        "stats": {
            "econ": 80,
            "dipl": 50,
            "govt": 70,
            "scty": 20
        }
    },
    {
        "name": "Democratic Socialism",
        "stats": {
            "econ": 80,
            "dipl": 50,
            "govt": 50,
            "scty": 80
        }
    },
    {
        "name": "Revolutionary Socialism",
        "stats": {
            "econ": 80,
            "dipl": 20,
            "govt": 50,
            "scty": 70
        }
    },
    {
        "name": "Libertarian Socialism",
        "stats": {
            "econ": 80,
            "dipl": 80,
            "govt": 80,
            "scty": 80
        }
    },
    {
        "name": "Anarcho-Syndicalism",
        "stats": {
            "econ": 80,
            "dipl": 50,
            "govt": 100,
            "scty": 80
        }
    },
    {
        "name": "Left-Wing Populism",
        "stats": {
            "econ": 60,
            "dipl": 40,
            "govt": 30,
            "scty": 70
        }
    },
    {
        "name": "Theocratic Distributism",
        "stats": {
            "econ": 60,
            "dipl": 40,
            "govt": 30,
            "scty": 20
        }
    },
    {
        "name": "Distributism",
        "stats": {
            "econ": 60,
            "dipl": 50,
            "govt": 50,
            "scty": 20
        }
    },
    {
        "name": "Social Liberalism",
        "stats": {
            "econ": 60,
            "dipl": 60,
            "govt": 60,
            "scty": 80
        }
    },
    {
        "name": "Christian Democracy",
        "stats": {
            "econ": 60,
            "dipl": 60,
            "govt": 50,
            "scty": 30
        }
    },
    {
        "name": "Social Democracy",
        "stats": {
            "econ": 60,
            "dipl": 70,
            "govt": 60,
            "scty": 80
        }
    },
    {
        "name": "Progressivism",
        "stats": {
            "econ": 60,
            "dipl": 80,
            "govt": 60,
            "scty": 100
        }
    },
    {
        "name": "Anarcho-Mutualism",
        "stats": {
            "econ": 60,
            "dipl": 50,
            "govt": 100,
            "scty": 70
        }
    },
    {
        "name": "National Totalitarianism",
        "stats": {
            "econ": 50,
            "dipl": 20,
            "govt": 0,
            "scty": 50
        }
    },
    {
        "name": "Global Totalitarianism",
        "stats": {
            "econ": 50,
            "dipl": 80,
            "govt": 0,
            "scty": 50
        }
    },
    {
        "name": "Technocracy",
        "stats": {
            "econ": 60,
            "dipl": 60,
            "govt": 20,
            "scty": 70
        }
    },
    {
        "name": "Centrist",
        "stats": {
            "econ": 50,
            "dipl": 50,
            "govt": 50,
            "scty": 50
        }
    },
    {
        "name": "Liberalism",
        "stats": {
            "econ": 50,
            "dipl": 60,
            "govt": 60,
            "scty": 60
        }
    },
    {
        "name": "Religious Anarchism",
        "stats": {
            "econ": 50,
            "dipl": 50,
            "govt": 100,
            "scty": 20
        }
    },
    {
        "name": "Right-Wing Populism",
        "stats": {
            "econ": 40,
            "dipl": 30,
            "govt": 30,
            "scty": 30
        }
    },
    {
        "name": "Moderate Conservatism",
        "stats": {
            "econ": 40,
            "dipl": 40,
            "govt": 50,
            "scty": 30
        }
    },
    {
        "name": "Reactionary",
        "stats": {
            "econ": 40,
            "dipl": 40,
            "govt": 40,
            "scty": 10
        }
    },
    {
        "name": "Social Libertarianism",
        "stats": {
            "econ": 60,
            "dipl": 70,
            "govt": 80,
            "scty": 70
        }
    },
    {
        "name": "Libertarianism",
        "stats": {
            "econ": 40,
            "dipl": 60,
            "govt": 80,
            "scty": 60
        }
    },
    {
        "name": "Anarcho-Egoism",
        "stats": {
            "econ": 40,
            "dipl": 50,
            "govt": 100,
            "scty": 50
        }
    },
    {
        "name": "Nazism",
        "stats": {
            "econ": 40,
            "dipl": 0,
            "govt": 0,
            "scty": 5
        }
    },
    {
        "name": "Autocracy",
        "stats": {
            "econ": 50,
            "dipl": 20,
            "govt": 20,
            "scty": 50
        }
    },
    {
        "name": "Fascism",
        "stats": {
            "econ": 40,
            "dipl": 20,
            "govt": 20,
            "scty": 20
        }
    },
    {
        "name": "Capitalist Fascism",
        "stats": {
            "econ": 20,
            "dipl": 20,
            "govt": 20,
            "scty": 20
        }
    },
    {
        "name": "Conservatism",
        "stats": {
            "econ": 30,
            "dipl": 40,
            "govt": 40,
            "scty": 20
        }
    },
    {
        "name": "Neo-Liberalism",
        "stats": {
            "econ": 30,
            "dipl": 30,
            "govt": 50,
            "scty": 60
        }
    },
    {
        "name": "Classical Liberalism",
        "stats": {
            "econ": 30,
            "dipl": 60,
            "govt": 60,
            "scty": 80
        }
    },
    {
        "name": "Authoritarian Capitalism",
        "stats": {
            "econ": 20,
            "dipl": 30,
            "govt": 20,
            "scty": 40
        }
    },
    {
        "name": "State Capitalism",
        "stats": {
            "econ": 20,
            "dipl": 50,
            "govt": 30,
            "scty": 50
        }
    },
    {
        "name": "Neo-Conservatism",
        "stats": {
            "econ": 20,
            "dipl": 20,
            "govt": 40,
            "scty": 20
        }
    },
    {
        "name": "Fundamentalism",
        "stats": {
            "econ": 20,
            "dipl": 30,
            "govt": 30,
            "scty": 5
        }
    },
    {
        "name": "Libertarian Capitalism",
        "stats": {
            "econ": 20,
            "dipl": 50,
            "govt": 80,
            "scty": 60
        }
    },
    {
        "name": "Market Anarchism",
        "stats": {
            "econ": 20,
            "dipl": 50,
            "govt": 100,
            "scty": 50
        }
    },
    {
        "name": "Objectivism",
        "stats": {
            "econ": 10,
            "dipl": 50,
            "govt": 90,
            "scty": 40
        }
    },
    {
        "name": "Totalitarian Capitalism",
        "stats": {
            "econ": 0,
            "dipl": 30,
            "govt": 0,
            "scty": 50
        }
    },
    {
        "name": "Ultra-Capitalism",
        "stats": {
            "econ": 0,
            "dipl": 40,
            "govt": 50,
            "scty": 50
        }
    },
    {
        "name": "Anarcho-Capitalism",
        "stats": {
            "econ": 0,
            "dipl": 50,
            "govt": 100,
            "scty": 50
        }
    }
]

assert len(IDEOLOGIES) == 52, f"expected 52 ideologies, got {len(IDEOLOGIES)}"


@dataclass
class EightValuesResult:
    # Axis percentages (0-100), both poles for convenience.
    econ_equality_pct: float
    econ_markets_pct: float
    dipl_peace_pct: float
    dipl_nation_pct: float
    govt_liberty_pct: float
    govt_authority_pct: float
    scty_progress_pct: float
    scty_tradition_pct: float

    # Axis labels (ported from results.html thresholds).
    econ_label: str
    dipl_label: str
    govt_label: str
    scty_label: str

    # Closest ideology by weighted distance (52-entry table).
    ideology: str

    # GusWorld-specific scalar: 0.0 = full economic Equality (left),
    # 1.0 = full economic Markets (right). Primary axis = Economic ONLY.
    rightness: float = field(repr=True)

    # Societal axis exposed separately for MANUAL tie-breaking
    # (Tradition = more right-coded). NOT part of `rightness` itself.
    societal_tradition_pct: float = field(repr=True)

    def as_dict(self) -> dict:
        return {
            "econ_equality_pct": self.econ_equality_pct,
            "econ_markets_pct": self.econ_markets_pct,
            "dipl_peace_pct": self.dipl_peace_pct,
            "dipl_nation_pct": self.dipl_nation_pct,
            "govt_liberty_pct": self.govt_liberty_pct,
            "govt_authority_pct": self.govt_authority_pct,
            "scty_progress_pct": self.scty_progress_pct,
            "scty_tradition_pct": self.scty_tradition_pct,
            "econ_label": self.econ_label,
            "dipl_label": self.dipl_label,
            "govt_label": self.govt_label,
            "scty_label": self.scty_label,
            "ideology": self.ideology,
            "rightness": self.rightness,
            "societal_tradition_pct": self.societal_tradition_pct,
        }


def _normalize_answer(raw: str) -> str:
    key = raw.strip().lower()
    if key not in ANSWER_MULTIPLIERS:
        raise ValueError(
            f"invalid answer token {raw!r}; expected one of "
            f"{sorted(ANSWER_MULTIPLIERS)} (case-insensitive)"
        )
    return key


def _calc_score(score: float, max_axis: float) -> float:
    """Port of calc_score() in quiz.html: 100*(max+score)/(2*max), 1 decimal."""
    if max_axis == 0:
        # Cannot happen with the official 70 questions (every axis has
        # nonzero-weighted questions), but guard defensively anyway.
        return 50.0
    return round(100.0 * (max_axis + score) / (2.0 * max_axis), 1)


def score_answers(answers: Sequence[str]) -> EightValuesResult:
    """Compute the 4-axis scores, labels, closest ideology, and rightness
    scalar from a sequence of exactly 70 answers (sa/a/n/d/sd, order-matched
    to QUESTIONS / PERGUNTAS.md).
    """
    if len(answers) != len(QUESTIONS):
        raise ValueError(
            f"expected exactly {len(QUESTIONS)} answers, got {len(answers)}"
        )

    normalized = [_normalize_answer(a) for a in answers]

    max_axis = {axis: 0.0 for axis in AXES}
    score_axis = {axis: 0.0 for axis in AXES}

    for question in QUESTIONS:
        for axis in AXES:
            max_axis[axis] += abs(question["effect"][axis])

    for question, answer in zip(QUESTIONS, normalized):
        mult = ANSWER_MULTIPLIERS[answer]
        for axis in AXES:
            score_axis[axis] += mult * question["effect"][axis]

    equality_pct = _calc_score(score_axis["econ"], max_axis["econ"])
    peace_pct = _calc_score(score_axis["dipl"], max_axis["dipl"])
    liberty_pct = _calc_score(score_axis["govt"], max_axis["govt"])
    progress_pct = _calc_score(score_axis["scty"], max_axis["scty"])

    markets_pct = round(100.0 - equality_pct, 1)
    nation_pct = round(100.0 - peace_pct, 1)
    authority_pct = round(100.0 - liberty_pct, 1)
    tradition_pct = round(100.0 - progress_pct, 1)

    econ_label = _axis_label(equality_pct, ECON_LABELS)
    dipl_label = _axis_label(peace_pct, DIPL_LABELS)
    govt_label = _axis_label(liberty_pct, GOVT_LABELS)
    scty_label = _axis_label(progress_pct, SCTY_LABELS)

    ideology = _closest_ideology(equality_pct, peace_pct, liberty_pct, progress_pct)

    # --- GusWorld rule: rightness is driven ONLY by the economic axis. ---
    rightness = round(markets_pct / 100.0, 3)

    return EightValuesResult(
        econ_equality_pct=equality_pct,
        econ_markets_pct=markets_pct,
        dipl_peace_pct=peace_pct,
        dipl_nation_pct=nation_pct,
        govt_liberty_pct=liberty_pct,
        govt_authority_pct=authority_pct,
        scty_progress_pct=progress_pct,
        scty_tradition_pct=tradition_pct,
        econ_label=econ_label,
        dipl_label=dipl_label,
        govt_label=govt_label,
        scty_label=scty_label,
        ideology=ideology,
        rightness=rightness,
        societal_tradition_pct=tradition_pct,
    )


def _closest_ideology(equality_pct: float, peace_pct: float, liberty_pct: float, progress_pct: float) -> str:
    """Port of the ideology-matching loop in results.html."""
    best_name = ""
    best_dist = float("inf")
    for ideology in IDEOLOGIES:
        stats = ideology["stats"]
        dist = 0.0
        dist += abs(stats["econ"] - equality_pct) ** 2
        dist += abs(stats["govt"] - liberty_pct) ** 2
        dist += abs(stats["dipl"] - peace_pct) ** 1.73856063
        dist += abs(stats["scty"] - progress_pct) ** 1.73856063
        if dist < best_dist:
            best_dist = dist
            best_name = ideology["name"]
    return best_name


def format_report(result: EightValuesResult) -> str:
    lines = [
        "=== 8values Engine (GusWorld port) -- Report ===",
        "",
        f"Economic   (Equality <-> Markets):  "
        f"Equality {result.econ_equality_pct:5.1f}%  |  Markets {result.econ_markets_pct:5.1f}%   "
        f"[{result.econ_label}]",
        f"Diplomatic (Peace    <-> Nation):    "
        f"Peace    {result.dipl_peace_pct:5.1f}%  |  Nation  {result.dipl_nation_pct:5.1f}%   "
        f"[{result.dipl_label}]",
        f"Civil/Govt (Liberty  <-> Authority):  "
        f"Liberty  {result.govt_liberty_pct:5.1f}%  |  Authority {result.govt_authority_pct:5.1f}%   "
        f"[{result.govt_label}]",
        f"Societal   (Progress <-> Tradition):  "
        f"Progress {result.scty_progress_pct:5.1f}%  |  Tradition {result.scty_tradition_pct:5.1f}%   "
        f"[{result.scty_label}]",
        "",
        f"Closest ideology (8values 52-entry table): {result.ideology}",
        "",
        f"RIGHTNESS SCALAR (0.0=Equality/left .. 1.0=Markets/right, "
        f"economic axis ONLY): {result.rightness:.3f}",
        f"  (desemate manual disponivel: societal_tradition_pct = "
        f"{result.societal_tradition_pct:.1f}  -- Tradition e mais a direita)",
        "",
        f"RIGHTNESS={result.rightness:.3f}",
    ]
    return "\n".join(lines)


def _load_answers_from_text(text: str) -> list[str]:
    data = json.loads(text)
    if isinstance(data, dict):
        if "answers" not in data:
            raise ValueError('JSON object input must have an "answers" key')
        data = data["answers"]
    if not isinstance(data, list):
        raise ValueError("input JSON must be a list of 70 answer tokens, or an object with an \"answers\" list")
    return [str(x) for x in data]


def main(argv: list[str]) -> int:
    if len(argv) > 1:
        path = argv[1]
        with open(path, "r", encoding="utf-8") as fh:
            text = fh.read()
    else:
        text = sys.stdin.read()
        if not text.strip():
            print(
                "usage: 8values_engine.py [answers.json]   "
                "(or pipe JSON via stdin)",
                file=sys.stderr,
            )
            return 2

    try:
        answers = _load_answers_from_text(text)
        result = score_answers(answers)
    except (ValueError, json.JSONDecodeError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1

    print(format_report(result))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
