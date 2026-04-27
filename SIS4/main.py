import random

from bb84_core import run_bb84
from e91_core import run_e91
from cascade import cascade_correct


def print_result(result: dict):

    print("\n" + "=" * 70)
    print(f"Protocol: {result.get('protocol')}")
    print(f"Status: {result.get('status')}")
    print("=" * 70)

    print(f"Initial count: {result.get('initial_count')}")
    print(f"Sifted key length: {result.get('sifted_key_length')}")
    print(f"Checked bits: {result.get('checked_bits')}")
    print(f"Errors found: {result.get('errors_found', 'N/A')}")

    error_rate = result.get("error_rate", 0)
    print(f"Error rate: {error_rate:.2%}")

    print(f"Final key length: {result.get('final_key_length')}")
    print(f"Efficiency: {result.get('efficiency')}%")
    print(f"Eve enabled: {result.get('eve_enabled')}")

    if "noise_rate" in result:
        print(f"Noise rate: {result.get('noise_rate'):.2%}")

    if "basis_match_rate" in result:
        print(f"Basis match rate: {result.get('basis_match_rate'):.2%}")

    if "chsh_s" in result:
        print(f"CHSH S value: {result.get('chsh_s')}")

    if "keys_match_before_amplification" in result:
        print(f"Keys match before amplification: {result.get('keys_match_before_amplification')}")

    if "final_keys_match" in result:
        print(f"Final keys match: {result.get('final_keys_match')}")

    final_key = result.get("final_key", "")
    if final_key:
        print(f"Final key preview: {final_key[:64]}")

    if result.get("eve_enabled"):
        print("\n--- Eve statistics ---")
        for key, value in result.items():
            if key.startswith("eve_"):
                print(f"{key}: {value}")


def test_bb84():
    """
    Тестирует BB84 без Eve и с Eve.
    """

    print("\n\nTEST 1: BB84 WITHOUT EVE")

    result_without_eve = run_bb84(
        num_photons=5000,
        check_percentage=0.1,
        error_threshold=0.11,
        eve_enabled=False,
        noise_rate=0.01
    )

    print_result(result_without_eve)

    print("\n\nTEST 2: BB84 WITH EVE")

    result_with_eve = run_bb84(
        num_photons=5000,
        check_percentage=0.1,
        error_threshold=0.11,
        eve_enabled=True,
        noise_rate=0.01
    )

    print_result(result_with_eve)


def test_e91():
    """
    Тестирует E91 без Eve и с Eve.
    """

    print("\n\nTEST 3: E91 WITHOUT EVE")

    result_without_eve = run_e91(
        num_pairs=5000,
        check_percentage=0.1,
        error_threshold=0.11,
        eve_enabled=False,
        noise_rate=0.01
    )

    print_result(result_without_eve)

    print("\n\nTEST 4: E91 WITH EVE")

    result_with_eve = run_e91(
        num_pairs=5000,
        check_percentage=0.1,
        error_threshold=0.11,
        eve_enabled=True,
        noise_rate=0.01
    )

    print_result(result_with_eve)


def test_cascade():
    """
    Отдельный тест Cascade error correction.

    Мы искусственно создаём два ключа:
    - Alice key — правильный
    - Bob key — с ошибками

    Затем Cascade пытается исправить Bob key.
    """

    print("\n\nTEST 5: CASCADE ERROR CORRECTION")

    n = 500
    simulated_error_rate = 0.05

    alice_key = [random.randint(0, 1) for _ in range(n)]

    bob_key = [
        bit ^ 1 if random.random() < simulated_error_rate else bit
        for bit in alice_key
    ]

    errors_before = sum(a != b for a, b in zip(alice_key, bob_key))

    print(f"Bits: {n}")
    print(f"Errors before Cascade: {errors_before}/{n} = {errors_before / n:.2%}")

    result = cascade_correct(
        alice_key=alice_key,
        bob_key=bob_key,
        estimated_error_rate=simulated_error_rate,
        num_passes=4
    )

    corrected_key = result["corrected_key"]
    errors_after = sum(a != b for a, b in zip(alice_key, corrected_key))

    print(f"Errors after Cascade: {errors_after}/{n} = {errors_after / n:.2%}")
    print(f"Corrections made: {result['corrections_made']}")
    print(f"Parity queries: {result['parity_queries']}")
    print(f"Remaining error rate: {result['remaining_error_rate']}")
    print(f"Pass block sizes: {result['pass_block_sizes']}")


def statistical_bb84_runs():
    """
    Запускает BB84 несколько раз,
    чтобы показать среднюю статистику.
    """

    print("\n\nTEST 6: STATISTICAL BB84 RUNS")

    runs = 20

    error_rates_without_eve = []
    error_rates_with_eve = []
    basis_match_rates = []
    detection_count = 0

    for _ in range(runs):
        result_without_eve = run_bb84(
            num_photons=2000,
            check_percentage=0.1,
            error_threshold=0.11,
            eve_enabled=False,
            noise_rate=0.01
        )

        result_with_eve = run_bb84(
            num_photons=2000,
            check_percentage=0.1,
            error_threshold=0.11,
            eve_enabled=True,
            noise_rate=0.01
        )

        error_rates_without_eve.append(result_without_eve["error_rate"])
        error_rates_with_eve.append(result_with_eve["error_rate"])
        basis_match_rates.append(result_without_eve.get("basis_match_rate", 0))

        if result_with_eve["status"] == "ABORTED_EAVESDROPPER_DETECTED":
            detection_count += 1

    avg_no_eve = sum(error_rates_without_eve) / runs
    avg_with_eve = sum(error_rates_with_eve) / runs
    avg_basis_match = sum(basis_match_rates) / runs
    detection_probability = detection_count / runs

    print(f"Runs: {runs}")
    print(f"Average error rate without Eve: {avg_no_eve:.2%}")
    print(f"Average error rate with Eve: {avg_with_eve:.2%}")
    print(f"Average basis match rate: {avg_basis_match:.2%}")
    print(f"Eve detection probability: {detection_probability:.2%}")


def main():
    print("SIS4 Quantum Key Distribution Testing")
    print("BB84 + E91 + Cascade")
    print("=" * 70)

    test_bb84()
    test_e91()
    test_cascade()
    statistical_bb84_runs()


if __name__ == "__main__":
    main()