"""
statistics.py — QKD Statistics & Analysis
Omar's part: Statistical functions used by both BB84 and E91 protocols.

Provides QBER analysis, key randomness tests, information-theoretic security
estimates, and comparison utilities for simulation results.
"""

import math
import random
from collections import Counter
from typing import Optional


# ── QBER Analysis ────────────────────────────────────────────────────────────

def compute_qber(alice_bits: list[int], bob_bits: list[int]) -> float:
    """
    Compute Quantum Bit Error Rate between Alice's and Bob's key bits.
    QBER = number_of_errors / total_bits
    """
    if not alice_bits:
        return 0.0
    errors = sum(a != b for a, b in zip(alice_bits, bob_bits))
    return errors / len(alice_bits)


def qber_to_security_level(qber: float) -> str:
    """
    Map QBER to a security classification:
    - < 1%   : Excellent
    - 1–5%   : Good
    - 5–11%  : Marginal (may indicate low noise/eavesdropping)
    - > 11%  : Insecure (BB84/E91 threshold)
    """
    if qber < 0.01:
        return "Excellent"
    elif qber < 0.05:
        return "Good"
    elif qber <= 0.11:
        return "Marginal"
    else:
        return "Insecure"


# ── Shannon Entropy ──────────────────────────────────────────────────────────

def binary_entropy(p: float) -> float:
    """
    Binary Shannon entropy H(p) = -p·log₂(p) - (1-p)·log₂(1-p).
    Used to quantify information leakage per bit at error rate p.
    """
    if p <= 0 or p >= 1:
        return 0.0
    return -p * math.log2(p) - (1 - p) * math.log2(1 - p)


def key_entropy(key_bits: list[int]) -> float:
    """
    Empirical Shannon entropy of a key bit string.
    H = -Σ pᵢ·log₂(pᵢ), where pᵢ is frequency of each symbol.
    Perfect random key → H ≈ 1.0 bit/bit.
    """
    if not key_bits:
        return 0.0
    counts = Counter(key_bits)
    total = len(key_bits)
    return -sum((c / total) * math.log2(c / total) for c in counts.values())


# ── Secure Key Length Estimate ───────────────────────────────────────────────

def estimate_secure_key_length(
    sifted_length: int,
    error_rate: float,
    leaked_bits: int = 0,
) -> int:
    """
    Estimate number of secret bits extractable via privacy amplification.
    Based on information-theoretic bound for BB84/E91:

        l_sec = n · (1 - 2·H(e)) - leaked_bits

    where n = sifted key length, e = QBER, H = binary entropy.
    Returns max(0, l_sec).
    """
    if sifted_length == 0:
        return 0
    h_e = binary_entropy(error_rate)
    secure = int(sifted_length * (1 - 2 * h_e)) - leaked_bits
    return max(0, secure)


# ── Randomness Tests ─────────────────────────────────────────────────────────

def frequency_test(key_bits: list[int]) -> dict:
    """
    NIST SP 800-22 Frequency (Monobit) Test.
    Tests whether the number of 1s and 0s are approximately equal.
    Returns {'ones_ratio': float, 'passed': bool}.
    A ratio near 0.5 indicates randomness.
    """
    if not key_bits:
        return {"ones_ratio": 0.0, "passed": False}
    ones = sum(key_bits)
    ratio = ones / len(key_bits)
    passed = 0.4 <= ratio <= 0.6
    return {"ones_ratio": round(ratio, 4), "passed": passed}


def runs_test(key_bits: list[int]) -> dict:
    """
    Simplified Runs Test: counts consecutive same-bit runs.
    Too few runs → clustering; too many → alternating pattern.
    Returns {'runs_count': int, 'expected_runs': float, 'passed': bool}.
    """
    if len(key_bits) < 2:
        return {"runs_count": 0, "expected_runs": 0.0, "passed": False}

    ones = sum(key_bits)
    n = len(key_bits)
    p = ones / n

    # Expected runs for a random sequence
    expected = 2 * n * p * (1 - p) + 1

    # Count actual runs
    runs = 1
    for i in range(1, n):
        if key_bits[i] != key_bits[i - 1]:
            runs += 1

    # Accept if within ±30% of expected
    passed = abs(runs - expected) / expected < 0.30 if expected > 0 else False
    return {
        "runs_count": runs,
        "expected_runs": round(expected, 2),
        "passed": passed,
    }


def block_frequency_test(key_bits: list[int], block_size: int = 8) -> dict:
    """
    Block Frequency Test: checks that each block of `block_size` bits
    has roughly equal 0s and 1s.
    Returns {'max_deviation': float, 'passed': bool}.
    """
    n = len(key_bits)
    if n < block_size:
        return {"max_deviation": 1.0, "passed": False}

    deviations = []
    for start in range(0, n - block_size + 1, block_size):
        block = key_bits[start:start + block_size]
        ratio = sum(block) / block_size
        deviations.append(abs(ratio - 0.5))

    max_dev = max(deviations) if deviations else 1.0
    return {
        "max_deviation": round(max_dev, 4),
        "passed": max_dev < 0.25,
    }


def run_all_randomness_tests(key_bits: list[int]) -> dict:
    """
    Run all randomness tests on the final key.
    Returns aggregated results.
    """
    freq  = frequency_test(key_bits)
    runs  = runs_test(key_bits)
    block = block_frequency_test(key_bits)
    ent   = key_entropy(key_bits)

    all_passed = freq["passed"] and runs["passed"] and block["passed"]

    return {
        "frequency_test": freq,
        "runs_test": runs,
        "block_frequency_test": block,
        "entropy_per_bit": round(ent, 4),
        "overall_randomness_ok": all_passed,
    }


# ── Protocol Efficiency ──────────────────────────────────────────────────────

def compute_efficiency(
    initial_count: int,
    final_key_length: int,
) -> float:
    """
    Key generation efficiency = final_key_length / initial_count * 100 (%).
    BB84 theoretical max ≈ 25%; E91 ≈ 16% (due to sifting).
    """
    if initial_count == 0:
        return 0.0
    return round((final_key_length / initial_count) * 100, 2)


# ── Comparison & Summary ─────────────────────────────────────────────────────

def compare_results(result_bb84: dict, result_e91: dict) -> dict:
    """
    Compare BB84 and E91 simulation results side by side.
    Returns a structured comparison dict.
    """
    def safe_get(d, key, default=None):
        return d.get(key, default)

    return {
        "protocol":          ["BB84", "E91"],
        "initial_count":     [safe_get(result_bb84, "initial_count"),
                               safe_get(result_e91, "initial_count")],
        "sifted_key_length": [safe_get(result_bb84, "sifted_key_length"),
                               safe_get(result_e91, "sifted_key_length")],
        "error_rate":        [safe_get(result_bb84, "error_rate"),
                               safe_get(result_e91, "error_rate")],
        "final_key_length":  [safe_get(result_bb84, "final_key_length"),
                               safe_get(result_e91, "final_key_length")],
        "efficiency_%":      [safe_get(result_bb84, "efficiency"),
                               safe_get(result_e91, "efficiency")],
        "status":            [safe_get(result_bb84, "status"),
                               safe_get(result_e91, "status")],
    }


def print_result_summary(result: dict) -> None:
    """Pretty-print a QKD simulation result dict."""
    proto = result.get("protocol", "?")
    print(f"\n{'='*50}")
    print(f"  {proto} Protocol Result")
    print(f"{'='*50}")
    print(f"  Status          : {result.get('status', '?')}")
    print(f"  Initial photons : {result.get('initial_count', '?')}")
    print(f"  Sifted key len  : {result.get('sifted_key_length', '?')}")
    print(f"  Checked bits    : {result.get('checked_bits', '?')}")
    print(f"  QBER            : {result.get('error_rate', '?'):.2%}")
    print(f"  Final key length: {result.get('final_key_length', '?')}")
    print(f"  Efficiency      : {result.get('efficiency', '?'):.1f}%")
    print(f"  Eve present     : {result.get('eve_enabled', '?')}")
    if "chsh_s" in result:
        print(f"  CHSH |S|        : {abs(result['chsh_s']):.4f} "
              f"({'> 2: quantum ✓' if abs(result['chsh_s']) > 2 else '≤ 2: classical ✗'})")
    print(f"{'='*50}\n")


# ── Monte Carlo Batch Statistics ─────────────────────────────────────────────

def batch_stats(results: list[dict]) -> dict:
    """
    Compute aggregate statistics over multiple simulation runs.
    Useful for performance benchmarking across many trials.
    """
    if not results:
        return {}

    def avg(key):
        vals = [r[key] for r in results if key in r]
        return round(sum(vals) / len(vals), 4) if vals else None

    def std(key):
        vals = [r[key] for r in results if key in r]
        if len(vals) < 2:
            return 0.0
        m = sum(vals) / len(vals)
        return round(math.sqrt(sum((v - m) ** 2 for v in vals) / len(vals)), 4)

    success_rate = sum(1 for r in results if r.get("success")) / len(results)

    return {
        "num_trials":        len(results),
        "success_rate":      round(success_rate, 4),
        "avg_error_rate":    avg("error_rate"),
        "std_error_rate":    std("error_rate"),
        "avg_efficiency":    avg("efficiency"),
        "avg_final_key_len": avg("final_key_length"),
        "avg_sifted_len":    avg("sifted_key_length"),
    }
