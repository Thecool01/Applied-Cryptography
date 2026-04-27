import random

from photon import Photon, random_bit, random_basis, measure_photon
from eve import Eve
from privacy import privacy_amplification, key_to_string


def apply_noise(bit: int, noise_rate: float) -> int:
    """
    Добавляет естественный шум канала.

    Если случайное число меньше noise_rate,
    бит меняется на противоположный.

    Например:
    0 -> 1
    1 -> 0
    """

    if random.random() < noise_rate:
        return 1 - bit

    return bit


def run_bb84(
        num_photons: int = 1000,
        check_percentage: float = 0.1,
        error_threshold: float = 0.11,
        eve_enabled: bool = False,
        noise_rate: float = 0.0
) -> dict:
    """
    Главная функция BB84 протокола.

    Этапы:
    1. Alice создаёт случайные биты и базисы.
    2. Alice кодирует биты в фотоны.
    3. Eve может перехватывать фотоны.
    4. Bob измеряет фотоны в случайных базисах.
    5. Alice и Bob сравнивают только базисы.
    6. Они оставляют только совпавшие базисы.
    7. Выполняется error checking.
    8. Если error rate высокий — протокол прерывается.
    9. Если всё безопасно — выполняется privacy amplification.
    """

    if num_photons <= 0:
        raise ValueError("Количество фотонов должно быть больше 0")

    if not 0 <= check_percentage <= 1:
        raise ValueError("check_percentage должен быть от 0 до 1")

    if not 0 <= error_threshold <= 1:
        raise ValueError("error_threshold должен быть от 0 до 1")

    if not 0 <= noise_rate <= 1:
        raise ValueError("noise_rate должен быть от 0 до 1")

    # Данные Alice
    alice_bits = []
    alice_bases = []

    # Данные Bob
    bob_bases = []
    bob_results = []

    # Журнал первых фотонов для будущей визуализации
    transmission_log = []

    # Создаём Eve, если она включена
    eve = Eve() if eve_enabled else None

    # -------------------------------
    # PHASE 1: Quantum Transmission
    # -------------------------------

    for i in range(num_photons):
        # Alice генерирует случайный бит и случайный базис
        alice_bit = random_bit()
        alice_basis = random_basis()

        # Alice создаёт фотон
        original_photon = Photon.create(alice_bit, alice_basis)

        alice_bits.append(alice_bit)
        alice_bases.append(alice_basis)

        # Фотон, который реально дойдёт до Bob
        transmitted_photon = original_photon

        # Если Eve включена, она перехватывает фотон
        if eve_enabled and eve is not None:
            transmitted_photon = eve.intercept_and_resend(original_photon)

        # Bob выбирает случайный базис
        bob_basis = random_basis()

        # Bob измеряет фотон
        bob_bit = measure_photon(transmitted_photon, bob_basis)

        # Добавляем естественный шум канала, если он есть
        bob_bit = apply_noise(bob_bit, noise_rate)

        bob_bases.append(bob_basis)
        bob_results.append(bob_bit)

        # Сохраняем первые 50 шагов для визуализации/отладки
        if i < 50:
            transmission_log.append({
                "index": i,
                "alice_bit": alice_bit,
                "alice_basis": alice_basis,
                "alice_polarization": original_photon.polarization,
                "bob_basis": bob_basis,
                "bob_result": bob_bit,
                "basis_matched": alice_basis == bob_basis
            })

    # -------------------------------
    # PHASE 2: Basis Reconciliation
    # -------------------------------

    alice_sifted_key = []
    bob_sifted_key = []

    for i in range(num_photons):
        # Alice и Bob публично сравнивают только базисы, не сами биты
        if alice_bases[i] == bob_bases[i]:
            alice_sifted_key.append(alice_bits[i])
            bob_sifted_key.append(bob_results[i])

    sifted_key_length = len(alice_sifted_key)
    basis_match_rate = sifted_key_length / num_photons

    # Если совпавших базисов нет, протокол не может продолжаться
    if sifted_key_length == 0:
        return {
            "protocol": "BB84",
            "success": False,
            "status": "FAILED_NO_MATCHED_BASES",
            "initial_count": num_photons,
            "sifted_key_length": 0,
            "checked_bits": 0,
            "errors_found": 0,
            "error_rate": 0.0,
            "basis_match_rate": basis_match_rate,
            "alice_final_key": "",
            "bob_final_key": "",
            "final_key": "",
            "final_key_preview": "",
            "final_key_length": 0,
            "final_keys_match": False,
            "efficiency": 0.0,
            "eve_enabled": eve_enabled,
            "noise_rate": noise_rate,
            "transmission_log": transmission_log
        }

    # -------------------------------
    # PHASE 3: Error Checking
    # -------------------------------

    check_count = int(sifted_key_length * check_percentage)

    # Минимум 1 бит для проверки, если sifted key не пустой
    if check_count == 0:
        check_count = 1

    # Защита, чтобы check_count не был больше длины ключа
    check_count = min(check_count, sifted_key_length)

    # Случайно выбираем индексы для проверки
    check_indices = random.sample(range(sifted_key_length), check_count)
    check_indices_set = set(check_indices)

    errors = 0

    for index in check_indices:
        if alice_sifted_key[index] != bob_sifted_key[index]:
            errors += 1

    error_rate = errors / check_count

    # Удаляем проверенные биты из ключа,
    # потому что они уже были публично раскрыты
    alice_remaining_key = []
    bob_remaining_key = []

    for i in range(sifted_key_length):
        if i not in check_indices_set:
            alice_remaining_key.append(alice_sifted_key[i])
            bob_remaining_key.append(bob_sifted_key[i])

    keys_match_before_amplification = alice_remaining_key == bob_remaining_key

    # Если error rate слишком высокий, протокол прерывается
    if error_rate > error_threshold:
        result = {
            "protocol": "BB84",
            "success": False,
            "status": "ABORTED_EAVESDROPPER_DETECTED",
            "initial_count": num_photons,
            "sifted_key_length": sifted_key_length,
            "checked_bits": check_count,
            "errors_found": errors,
            "error_rate": error_rate,
            "basis_match_rate": basis_match_rate,
            "keys_match_before_amplification": keys_match_before_amplification,
            "alice_final_key": "",
            "bob_final_key": "",
            "final_key": "",
            "final_key_preview": "",
            "final_key_length": 0,
            "final_keys_match": False,
            "efficiency": 0.0,
            "eve_enabled": eve_enabled,
            "noise_rate": noise_rate,
            "transmission_log": transmission_log
        }

        if eve_enabled and eve is not None:
            result.update(eve.get_statistics())

        return result

    # -------------------------------
    # PHASE 4: Privacy Amplification
    # -------------------------------

    # Важно: privacy amplification делаем отдельно для Alice и Bob
    alice_final_key_bits = privacy_amplification(alice_remaining_key)
    bob_final_key_bits = privacy_amplification(bob_remaining_key)

    alice_final_key = key_to_string(alice_final_key_bits)
    bob_final_key = key_to_string(bob_final_key_bits)

    final_keys_match = alice_final_key == bob_final_key

    final_key_length = len(alice_final_key_bits)
    efficiency = (final_key_length / num_photons) * 100

    result = {
        "protocol": "BB84",
        "success": final_keys_match,
        "status": "SECURE" if final_keys_match else "FAILED_KEY_MISMATCH",
        "initial_count": num_photons,
        "sifted_key_length": sifted_key_length,
        "checked_bits": check_count,
        "errors_found": errors,
        "error_rate": error_rate,
        "basis_match_rate": basis_match_rate,
        "keys_match_before_amplification": keys_match_before_amplification,
        "alice_final_key": alice_final_key,
        "bob_final_key": bob_final_key,
        "final_key": alice_final_key,
        "final_key_preview": alice_final_key[:64],
        "final_key_length": final_key_length,
        "final_keys_match": final_keys_match,
        "efficiency": efficiency,
        "eve_enabled": eve_enabled,
        "noise_rate": noise_rate,
        "transmission_log": transmission_log
    }

    if eve_enabled and eve is not None:
        result.update(eve.get_statistics())

    return result