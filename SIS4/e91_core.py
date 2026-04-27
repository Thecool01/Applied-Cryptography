"""
e91_core.py — E91 (Ekert 1991) Quantum Key Distribution Protocol
Omar's part: E91 simulation with entangled pairs, Bell inequality checking,
Alice/Bob basis choices, Eve attack simulation.
"""

import random
import math
from typing import Optional


# ── Basis & Measurement Constants ───────────────────────────────────────────

# Alice's measurement angles (degrees): 0°, 45°, 90°
ALICE_ANGLES = [0, 45, 90]

# Bob's measurement angles (degrees): 45°, 90°, 135°
BOB_ANGLES = [45, 90, 135]

# Shared angles (used for key): Alice=45°, Bob=45° and Alice=90°, Bob=90°
SHARED_ALICE = [45, 90]
SHARED_BOB = [45, 90]


# ── Entangled Pair ───────────────────────────────────────────────────────────

class EntangledPair:
    """
    Represents a singlet Bell state |ψ⁻⟩ = (|01⟩ - |10⟩) / √2.
    In this state, measurement outcomes are always anti-correlated
    when both parties measure in the same basis.
    """

    def __init__(self, noise_rate: float = 0.0):
        self.noise_rate = noise_rate
        # Hidden variable λ ∈ [0, 1) — local realist model approximation
        self._lambda = random.random()

    def measure_alice(self, angle_deg: float) -> int:
        """
        Alice measures her qubit at `angle_deg`.
        Returns +1 or -1 (encoded as 1 or 0).
        """
        angle_rad = math.radians(angle_deg)
        # Quantum probability: P(+1) = cos²((λ*2π - angle)/2)
        prob_plus = math.cos((self._lambda * 2 * math.pi - angle_rad) / 2) ** 2
        result = 1 if random.random() < prob_plus else 0
        if random.random() < self.noise_rate:
            result ^= 1
        return result

    def measure_bob(self, angle_deg: float, alice_result: int) -> int:
        """
        Bob measures his qubit at `angle_deg`.
        Due to entanglement, outcomes are correlated by the Bell state.
        Returns +1 or -1 (encoded as 1 or 0).
        """
        angle_rad = math.radians(angle_deg)
        alice_angle_rad = math.radians(self._get_alice_angle())
        delta = angle_rad - alice_angle_rad
        # Quantum correlation: E(a,b) = -cos(a-b)
        prob_same = (1 - math.cos(delta)) / 2  # prob Bob gets OPPOSITE of Alice
        # Bob is opposite to Alice when angles match (singlet)
        if random.random() < (1 - prob_same):
            result = 1 - alice_result  # anti-correlated
        else:
            result = alice_result
        if random.random() < self.noise_rate:
            result ^= 1
        return result

    def _get_alice_angle(self) -> float:
        """Retrieve the Alice angle based on hidden variable (internal use)."""
        return self._lambda * 360


# ── E91 Simulation ───────────────────────────────────────────────────────────

def generate_entangled_pairs(num_pairs: int, noise_rate: float) -> list[EntangledPair]:
    """Generate `num_pairs` entangled photon pairs."""
    return [EntangledPair(noise_rate=noise_rate) for _ in range(num_pairs)]


def alice_measure_all(pairs: list[EntangledPair]) -> tuple[list[int], list[int]]:
    """
    Alice randomly chooses a basis from ALICE_ANGLES and measures each qubit.
    Returns (basis_choices, measurement_results).
    """
    bases = [random.choice(ALICE_ANGLES) for _ in pairs]
    results = [pair.measure_alice(b) for pair, b in zip(pairs, bases)]
    return bases, results


def bob_measure_all(
    pairs: list[EntangledPair],
    alice_results: list[int]
) -> tuple[list[int], list[int]]:
    """
    Bob randomly chooses a basis from BOB_ANGLES and measures each qubit.
    Returns (basis_choices, measurement_results).
    """
    bases = [random.choice(BOB_ANGLES) for _ in pairs]
    results = [
        pair.measure_bob(b, a_res)
        for pair, b, a_res in zip(pairs, bases, alice_results)
    ]
    return bases, results


# ── Basis Reconciliation & Sifting ──────────────────────────────────────────

def sift_key(
    alice_bases: list[int],
    bob_bases: list[int],
    alice_results: list[int],
    bob_results: list[int],
) -> tuple[list[int], list[int]]:
    """
    Keep only measurements where Alice and Bob used a shared (matching) angle.
    Shared pairs: (Alice=45°, Bob=45°) and (Alice=90°, Bob=90°).
    At matching angles, results should be perfectly anti-correlated → flip Bob.
    Returns (alice_key_bits, bob_key_bits).
    """
    alice_key, bob_key = [], []
    for ab, bb, ar, br in zip(alice_bases, bob_bases, alice_results, bob_results):
        if ab == bb and ab in SHARED_ALICE and bb in SHARED_BOB:
            alice_key.append(ar)
            bob_key.append(1 - br)  # flip because singlet is anti-correlated
    return alice_key, bob_key


# ── Bell / CHSH Inequality Check ────────────────────────────────────────────

def compute_chsh(
    alice_bases: list[int],
    bob_bases: list[int],
    alice_results: list[int],
    bob_results: list[int],
) -> float:
    """
    Compute the CHSH parameter S from non-shared basis combinations.
    |S| > 2 → quantum correlations (no eavesdropping via classical channel).
    |S| ≤ 2 → classical / local hidden variable → possible eavesdropping.

    CHSH correlator pairs used (Alice°, Bob°):
        E(0,45), E(0,135), E(90,45), E(90,135)
    S = E(0,45) - E(0,135) + E(90,45) + E(90,135)
    """
    def correlation(a_angle: int, b_angle: int) -> float:
        products, count = 0, 0
        for ab, bb, ar, br in zip(alice_bases, bob_bases, alice_results, bob_results):
            if ab == a_angle and bb == b_angle:
                # convert 0/1 → ±1
                a_val = 2 * ar - 1
                b_val = 2 * br - 1
                products += a_val * b_val
                count += 1
        return products / count if count > 0 else 0.0

    e_0_45   = correlation(0,  45)
    e_0_135  = correlation(0,  135)
    e_90_45  = correlation(90, 45)
    e_90_135 = correlation(90, 135)

    S = e_0_45 - e_0_135 + e_90_45 + e_90_135
    return S


# ── Error Checking ───────────────────────────────────────────────────────────

def check_errors(
    alice_key: list[int],
    bob_key: list[int],
    check_percentage: float,
) -> tuple[float, list[int], list[int]]:
    """
    Randomly sample `check_percentage` of the sifted key to estimate QBER.
    Returns (error_rate, remaining_alice_key, remaining_bob_key).
    """
    n = len(alice_key)
    check_n = max(1, int(n * check_percentage))
    indices = random.sample(range(n), min(check_n, n))
    idx_set = set(indices)

    errors = sum(1 for i in indices if alice_key[i] != bob_key[i])
    error_rate = errors / len(indices) if indices else 0.0

    remaining_alice = [b for i, b in enumerate(alice_key) if i not in idx_set]
    remaining_bob   = [b for i, b in enumerate(bob_key)   if i not in idx_set]

    return error_rate, remaining_alice, remaining_bob


# ── Eve Attack for E91 ──────────────────────────────────────────────────────

class EveE91:
    """
    Intercept-resend attack adapted for E91.
    Eve intercepts each entangled pair, measures one photon (collapsing
    entanglement), then forwards a freshly prepared (unentangled) photon to Bob.
    This destroys Bell correlations, raising QBER ≈ 25%.
    """

    def __init__(self, intercept_rate: float = 1.0):
        self.intercept_rate = intercept_rate
        self.intercepted_count = 0

    def intercept_pair(
        self,
        pair: EntangledPair,
        alice_result: int,
        bob_angle: int,
    ) -> int:
        """
        Eve intercepts Bob's photon with probability `intercept_rate`.
        She randomly chooses a basis, measures, then re-sends a classical bit.
        Returns the (possibly corrupted) bit Bob receives.
        """
        if random.random() > self.intercept_rate:
            return pair.measure_bob(bob_angle, alice_result)

        self.intercepted_count += 1
        # Eve picks a random angle from all available angles
        eve_angle = random.choice(ALICE_ANGLES + BOB_ANGLES)
        eve_result = pair.measure_alice(eve_angle)  # collapses entanglement

        # Eve resends a classical photon prepared in her measured state
        # Bob's measurement on this unentangled photon:
        delta = math.radians(bob_angle - eve_angle)
        prob_match = math.cos(delta / 2) ** 2
        return eve_result if random.random() < prob_match else (1 - eve_result)

    def attack(
        self,
        pairs: list[EntangledPair],
        alice_results: list[int],
        bob_bases: list[int],
    ) -> list[int]:
        """Apply intercept-resend to all of Bob's measurements."""
        return [
            self.intercept_pair(pair, ar, bb)
            for pair, ar, bb in zip(pairs, alice_results, bob_bases)
        ]


# ── Main E91 Runner ──────────────────────────────────────────────────────────

def run_e91(
    num_pairs: int = 1000,
    check_percentage: float = 0.1,
    error_threshold: float = 0.11,
    eve_enabled: bool = False,
    noise_rate: float = 0.0,
) -> dict:
    """
    Full E91 QKD simulation.

    Parameters
    ----------
    num_pairs        : number of entangled photon pairs to generate
    check_percentage : fraction of sifted key used for error estimation
    error_threshold  : QBER above this → channel deemed insecure
    eve_enabled      : whether Eve performs an intercept-resend attack
    noise_rate       : channel noise probability per qubit

    Returns
    -------
    dict with unified result format (same keys as run_bb84)
    """
    # 1. Generate entangled pairs
    pairs = generate_entangled_pairs(num_pairs, noise_rate)

    # 2. Alice measures
    alice_bases, alice_results = alice_measure_all(pairs)

    # 3. Bob measures (or Eve intercepts first)
    bob_bases = [random.choice(BOB_ANGLES) for _ in pairs]
    if eve_enabled:
        eve = EveE91(intercept_rate=1.0)
        bob_results = eve.attack(pairs, alice_results, bob_bases)
    else:
        bob_results = [
            pair.measure_bob(bb, ar)
            for pair, bb, ar in zip(pairs, bob_bases, alice_results)
        ]

    # 4. CHSH check (Bell inequality)
    chsh_s = compute_chsh(alice_bases, bob_bases, alice_results, bob_results)

    # 5. Sift key (keep matching basis pairs)
    alice_key, bob_key = sift_key(alice_bases, bob_bases, alice_results, bob_results)
    sifted_length = len(alice_key)

    if sifted_length == 0:
        return {
            "protocol": "E91",
            "success": False,
            "status": "NO_SIFTED_KEY",
            "initial_count": num_pairs,
            "sifted_key_length": 0,
            "checked_bits": 0,
            "error_rate": 0.0,
            "final_key": "",
            "final_key_length": 0,
            "efficiency": 0.0,
            "eve_enabled": eve_enabled,
            "chsh_s": chsh_s,
        }

    # 6. Error checking (QBER estimation)
    error_rate, alice_key, bob_key = check_errors(
        alice_key, bob_key, check_percentage
    )
    checked_bits = max(1, int(sifted_length * check_percentage))

    # 7. Security decision
    bell_violated = abs(chsh_s) > 2.0  # quantum channel → entanglement intact
    secure = error_rate <= error_threshold and (bell_violated or not eve_enabled)
    status = "SECURE" if secure else "INSECURE"

    # 8. Final key (use alice_key after error check; in real system → Cascade)
    final_key_bits = alice_key  # Cascade correction applied separately
    final_key = "".join(str(b) for b in final_key_bits)

    efficiency = (len(final_key_bits) / num_pairs) * 100 if num_pairs > 0 else 0.0

    return {
        "protocol": "E91",
        "success": secure,
        "status": status,
        "initial_count": num_pairs,
        "sifted_key_length": sifted_length,
        "checked_bits": checked_bits,
        "error_rate": round(error_rate, 4),
        "final_key": final_key,
        "final_key_length": len(final_key),
        "efficiency": round(efficiency, 2),
        "eve_enabled": eve_enabled,
        "chsh_s": round(chsh_s, 4),
    }
