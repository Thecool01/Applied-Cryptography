import random

from photon import Photon, random_bit, random_basis, measure_photon
from eve import Eve
from privacy import privacy_amplification, key_to_string

"""
Добавляет естественный шум канала.

Если случайное число меньше noise_rate,
бит меняется на противоположный.

Например:
0 -> 1
1 -> 0
"""

def apply_noise(bit: int, noise_rate: float) -> int:
    if random.random() < noise_rate:
        return 1 - bit

    return bit

"""
   Главная функция BB84 протокола.

   Она выполняет все этапы:
   1. Alice создаёт случайные биты и базисы.
   2. Alice отправляет фотоны.
   3. Eve может перехватывать фотоны.
   4. Bob измеряет фотоны.
   5. Alice и Bob сравнивают базисы.
   6. Выполняется error checking.
   7. Выполняется privacy amplification.
   8. Возвращается итоговая статистика.
"""

def run_bb84(
        num_photons: int = 1000,
        check_percentage: float = 0.1,
        error_threshold: float = 0.11,
        eve_enabled: bool = False,
        noise_rate: float = 0.0
) -> dict:

    # Данные Alice
    alice_bits = []
    alice_bases = []
    photons = []

    # Данные Bob
    bob_bases = []
    bob_results = []

    # Создаём Eve, если она включена
    eve = Eve() if eve_enabled else None

    # -------------------------------
    # PHASE 1: Quantum Transmission
    # -------------------------------

    for _ in range(num_photons):
        # Alice генерирует случайный бит и случайный базис
        alice_bit = random_bit()
        alice_basis = random_basis()

        # Alice создаёт фотон
        photon = Photon.create(alice_bit, alice_basis)

        alice_bits.append(alice_bit)
        alice_bases.append(alice_basis)
        photons.append(photon)

        # Если Eve включена, она перехватывает фотон
        if eve_enabled and eve is not None:
            photon = eve.intercept_and_resend(photon)

        # Bob выбирает случайный базис
        bob_basis = random_basis()

        # Bob измеряет фотон
        bob_bit = measure_photon(photon, bob_basis)

        # Добавляем шум канала, если он есть
        bob_bit = apply_noise(bob_bit, noise_rate)

        bob_bases.append(bob_basis)
        bob_results.append(bob_bit)

    # -------------------------------
    # PHASE 2: Basis Reconciliation
    # -------------------------------

    alice_sifted_key = []
    bob_sifted_key = []
    matched_indices = []

    for i in range(num_photons):
        # Оставляем только те биты, где базисы Alice и Bob совпали
        if alice_bases[i] == bob_bases[i]:
            alice_sifted_key.append(alice_bits[i])
            bob_sifted_key.append(bob_results[i])
            matched_indices.append(i)

    sifted_key_length = len(alice_sifted_key)

    # Если совпавших базисов нет, протокол не может продолжаться
    if sifted_key_length == 0:
        return {
            "protocol": "BB84",
            "success": False,
            "status": "FAILED_NO_MATCHED_BASES",
            "initial_count": num_photons,
            "sifted_key_length": 0,
            "checked_bits": 0,
            "error_rate": 0,
            "final_key": "",
            "final_key_length": 0,
            "efficiency": 0,
            "eve_enabled": eve_enabled,
            "noise_rate": noise_rate
        }

    # -------------------------------
    # PHASE 3: Error Checking
    # -------------------------------

    check_count = int(sifted_key_length * check_percentage)

    # Минимум 1 бит для проверки, если sifted key не пустой
    if check_count == 0:
        check_count = 1

    # Случайно выбираем индексы для проверки
    check_indices = random.sample(range(sifted_key_length), check_count)

    errors = 0

    for index in check_indices:
        if alice_sifted_key[index] != bob_sifted_key[index]:
            errors += 1

    error_rate = errors / check_count

    # Удаляем проверенные биты из ключа
    alice_remaining_key = []
    bob_remaining_key = []

    for i in range(sifted_key_length):
        if i not in check_indices:
            alice_remaining_key.append(alice_sifted_key[i])
            bob_remaining_key.append(bob_sifted_key[i])

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
            "final_key": "",
            "final_key_length": 0,
            "efficiency": 0,
            "basis_match_rate": sifted_key_length / num_photons,
            "eve_enabled": eve_enabled,
            "noise_rate": noise_rate
        }

        if eve_enabled and eve is not None:
            result.update(eve.get_statistics())

        return result

    # -------------------------------
    # PHASE 4: Privacy Amplification
    # -------------------------------

    # Упрощённая privacy amplification через XOR пар битов
    final_key_bits = privacy_amplification(alice_remaining_key)

    final_key = key_to_string(final_key_bits)
    final_key_length = len(final_key_bits)

    efficiency = (final_key_length / num_photons) * 100

    # Проверяем, совпадают ли ключи Alice и Bob до privacy amplification
    keys_match_before_amplification = alice_remaining_key == bob_remaining_key

    result = {
        "protocol": "BB84",
        "success": True,
        "status": "SECURE",
        "initial_count": num_photons,
        "sifted_key_length": sifted_key_length,
        "checked_bits": check_count,
        "errors_found": errors,
        "error_rate": error_rate,
        "final_key": final_key,
        "final_key_preview": final_key[:64],
        "final_key_length": final_key_length,
        "efficiency": efficiency,
        "basis_match_rate": sifted_key_length / num_photons,
        "keys_match_before_amplification": keys_match_before_amplification,
        "eve_enabled": eve_enabled,
        "noise_rate": noise_rate
    }

    if eve_enabled and eve is not None:
        result.update(eve.get_statistics())

    return result