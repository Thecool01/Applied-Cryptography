import random

from photon import Photon, measure_photon
from bb84_core import run_bb84

try:
    from e91_core import run_e91
except ImportError:
    run_e91 = None

try:
    from cascade import cascade_correct
except ImportError:
    cascade_correct = None


def print_test_result(test_name: str, passed: bool, details: str = ""):
    """
    Красиво выводит результат теста.
    """

    status = "PASSED" if passed else "FAILED"
    print(f"[{status}] {test_name}")

    if details:
        print(f"        {details}")


def same_basis_test():
    """
    Same Basis Test:
    если Alice и Bob используют одинаковый базис,
    Bob должен получить правильный бит в 100% случаев.
    """

    trials = 1000
    errors = 0

    for _ in range(trials):
        bit = random.randint(0, 1)
        basis = random.choice(["+", "x"])

        photon = Photon.create(bit, basis)
        measured = measure_photon(photon, basis)

        if measured != bit:
            errors += 1

    passed = errors == 0
    print_test_result(
        "Same Basis Test",
        passed,
        f"errors={errors}, trials={trials}"
    )


def different_basis_test():
    """
    Different Basis Test:
    если Bob измеряет в другом базисе,
    совпадение должно быть примерно 50%.
    """

    trials = 5000
    matches = 0

    for _ in range(trials):
        bit = random.randint(0, 1)
        alice_basis = random.choice(["+", "x"])
        bob_basis = "x" if alice_basis == "+" else "+"

        photon = Photon.create(bit, alice_basis)
        measured = measure_photon(photon, bob_basis)

        if measured == bit:
            matches += 1

    match_rate = matches / trials

    passed = 0.45 <= match_rate <= 0.55

    print_test_result(
        "Different Basis Test",
        passed,
        f"match_rate={match_rate:.2%}, expected≈50%"
    )


def basis_match_rate_test():
    """
    Basis Match Rate Test:
    при случайном выборе базисов совпадение должно быть около 50%.
    """

    result = run_bb84(
        num_photons=10000,
        check_percentage=0.1,
        error_threshold=0.11,
        eve_enabled=False,
        noise_rate=0.0
    )

    basis_match_rate = result.get("basis_match_rate", 0)
    passed = 0.45 <= basis_match_rate <= 0.55

    print_test_result(
        "Basis Match Rate Test",
        passed,
        f"basis_match_rate={basis_match_rate:.2%}, expected≈50%"
    )


def no_eavesdropper_test():
    """
    No Eavesdropper Test:
    без Eve error rate должен быть низким,
    а протокол должен завершиться успешно.
    """

    result = run_bb84(
        num_photons=5000,
        check_percentage=0.1,
        error_threshold=0.11,
        eve_enabled=False,
        noise_rate=0.01
    )

    error_rate = result["error_rate"]
    success = result["success"]
    final_keys_match = result.get("final_keys_match", True)

    passed = error_rate <= 0.05 and success and final_keys_match

    print_test_result(
        "No Eavesdropper Test",
        passed,
        f"error_rate={error_rate:.2%}, success={success}, final_keys_match={final_keys_match}"
    )


def with_eavesdropper_test():
    """
    With Eavesdropper Test:
    с Eve error rate должен быть примерно 25%,
    и протокол должен abort при threshold 11%.
    """

    result = run_bb84(
        num_photons=10000,
        check_percentage=0.15,
        error_threshold=0.11,
        eve_enabled=True,
        noise_rate=0.0
    )

    error_rate = result["error_rate"]
    status = result["status"]

    passed = 0.15 <= error_rate <= 0.35 and status == "ABORTED_EAVESDROPPER_DETECTED"

    print_test_result(
        "With Eavesdropper Test",
        passed,
        f"error_rate={error_rate:.2%}, status={status}, expected≈25% and abort"
    )


def key_agreement_test():
    """
    Key Agreement Test:
    если протокол успешен, Alice и Bob должны получить одинаковый финальный ключ.
    """

    result = run_bb84(
        num_photons=5000,
        check_percentage=0.1,
        error_threshold=0.11,
        eve_enabled=False,
        noise_rate=0.0
    )

    final_keys_match = result.get("final_keys_match", False)
    success = result.get("success", False)

    passed = success and final_keys_match

    print_test_result(
        "Key Agreement Test",
        passed,
        f"success={success}, final_keys_match={final_keys_match}"
    )


def scalability_test():
    """
    Scalability Test:
    проверяем работу на 100, 1000, 10000 фотонах.
    """

    sizes = [100, 1000, 10000]
    passed_all = True
    details = []

    for size in sizes:
        result = run_bb84(
            num_photons=size,
            check_percentage=0.1,
            error_threshold=0.11,
            eve_enabled=False,
            noise_rate=0.01
        )

        ok = result["sifted_key_length"] > 0 and result["final_key_length"] >= 0
        passed_all = passed_all and ok

        details.append(
            f"{size}: status={result['status']}, "
            f"sifted={result['sifted_key_length']}, "
            f"final={result['final_key_length']}"
        )

    print_test_result(
        "Scalability Test",
        passed_all,
        "; ".join(details)
    )


def edge_case_test():
    """
    Edge Case Test:
    проверка минимального количества фотонов.
    """

    result = run_bb84(
        num_photons=10,
        check_percentage=0.1,
        error_threshold=0.11,
        eve_enabled=False,
        noise_rate=0.0
    )

    passed = "status" in result and result["initial_count"] == 10

    print_test_result(
        "Edge Case Test",
        passed,
        f"status={result['status']}, sifted={result['sifted_key_length']}"
    )


def multiple_run_test():
    """
    Multiple Run Test:
    запускаем BB84 100 раз и считаем средние значения.
    """

    runs = 100
    basis_rates = []
    error_without_eve = []
    error_with_eve = []
    detection_count = 0

    for _ in range(runs):
        result_no_eve = run_bb84(
            num_photons=1000,
            check_percentage=0.1,
            error_threshold=0.11,
            eve_enabled=False,
            noise_rate=0.01
        )

        result_with_eve = run_bb84(
            num_photons=1000,
            check_percentage=0.1,
            error_threshold=0.11,
            eve_enabled=True,
            noise_rate=0.0
        )

        basis_rates.append(result_no_eve.get("basis_match_rate", 0))
        error_without_eve.append(result_no_eve["error_rate"])
        error_with_eve.append(result_with_eve["error_rate"])

        if result_with_eve["status"] == "ABORTED_EAVESDROPPER_DETECTED":
            detection_count += 1

    avg_basis = sum(basis_rates) / runs
    avg_error_no_eve = sum(error_without_eve) / runs
    avg_error_with_eve = sum(error_with_eve) / runs
    detection_probability = detection_count / runs

    passed = (
            0.45 <= avg_basis <= 0.55
            and avg_error_no_eve <= 0.05
            and 0.15 <= avg_error_with_eve <= 0.35
            and detection_probability >= 0.90
    )

    print_test_result(
        "Multiple Run Statistical Test",
        passed,
        (
            f"avg_basis={avg_basis:.2%}, "
            f"avg_error_no_eve={avg_error_no_eve:.2%}, "
            f"avg_error_with_eve={avg_error_with_eve:.2%}, "
            f"detection_probability={detection_probability:.2%}"
        )
    )


def information_leakage_test():
    """
    Information Leakage Analysis:
    показываем сколько битов Eve примерно узнала
    и как privacy amplification уменьшает длину финального ключа.
    """

    result = run_bb84(
        num_photons=5000,
        check_percentage=0.1,
        error_threshold=0.50,
        eve_enabled=True,
        noise_rate=0.0
    )

    eve_learned_bits = result.get("eve_learned_bits", 0)
    sifted_key_length = result.get("sifted_key_length", 0)
    final_key_length = result.get("final_key_length", 0)

    # Здесь threshold специально высокий, чтобы протокол не abort,
    # и мы могли показать эффект privacy amplification.
    passed = final_key_length <= sifted_key_length

    print_test_result(
        "Information Leakage Analysis",
        passed,
        (
            f"eve_learned_bits={eve_learned_bits}, "
            f"sifted_key_length={sifted_key_length}, "
            f"final_key_length_after_privacy_amplification={final_key_length}"
        )
    )


def e91_bonus_test():
    """
    E91 Bonus Test:
    проверяем, что E91 запускается и возвращает основные метрики.
    """

    if run_e91 is None:
        print_test_result("E91 Bonus Test", False, "e91_core.py not found")
        return

    result = run_e91(
        num_pairs=1000,
        check_percentage=0.1,
        error_threshold=0.11,
        eve_enabled=False,
        noise_rate=0.01
    )

    passed = (
            result.get("protocol") == "E91"
            and "chsh_s" in result
            and "final_key_length" in result
    )

    print_test_result(
        "E91 Bonus Test",
        passed,
        f"status={result.get('status')}, chsh_s={result.get('chsh_s')}"
    )


def cascade_bonus_test():
    """
    Cascade Bonus Test:
    искусственно создаём ошибки в Bob key,
    затем проверяем, уменьшает ли Cascade количество ошибок.
    """

    if cascade_correct is None:
        print_test_result("Cascade Bonus Test", False, "cascade.py not found")
        return

    n = 500
    error_rate = 0.05

    alice_key = [random.randint(0, 1) for _ in range(n)]
    bob_key = [
        bit ^ 1 if random.random() < error_rate else bit
        for bit in alice_key
    ]

    errors_before = sum(a != b for a, b in zip(alice_key, bob_key))

    result = cascade_correct(
        alice_key=alice_key,
        bob_key=bob_key,
        estimated_error_rate=error_rate,
        num_passes=4
    )

    corrected_key = result["corrected_key"]
    errors_after = sum(a != b for a, b in zip(alice_key, corrected_key))

    passed = errors_after <= errors_before

    print_test_result(
        "Cascade Bonus Test",
        passed,
        f"errors_before={errors_before}, errors_after={errors_after}, parity_queries={result['parity_queries']}"
    )


def run_all_tests():
    """
    Запускает все тесты проекта.
    """

    print("=" * 70)
    print("SIS4 QKD TEST SUITE")
    print("=" * 70)

    same_basis_test()
    different_basis_test()
    basis_match_rate_test()

    no_eavesdropper_test()
    with_eavesdropper_test()
    key_agreement_test()

    scalability_test()
    edge_case_test()
    multiple_run_test()
    information_leakage_test()

    e91_bonus_test()
    cascade_bonus_test()

    print("=" * 70)
    print("Testing completed.")
    print("=" * 70)


if __name__ == "__main__":
    run_all_tests()