from photon import Photon, random_basis, measure_photon

"""
    Класс перехватчика Eve.
    
    Eve реализует intercept-resend attack:
    1. Перехватывает фотон Alice.
    2. Случайно выбирает базис измерения.
    3. Измеряет фотон.
    4. Создаёт новый фотон на основе своего результата.
    5. Отправляет новый фотон Bob.
"""


class Eve:

    def __init__(self):
        self.intercepted_count = 0
        self.basis_matches_with_alice = 0
        self.learned_bits = 0

    """
        Перехват и повторная отправка фотона.

        photon — оригинальный фотон от Alice.

        Возвращает новый фотон, который Eve отправляет Bob.
    """

    def intercept_and_resend(self, photon: Photon) -> Photon:


        self.intercepted_count += 1

        # Eve не знает базис Alice, поэтому выбирает случайный базис
        eve_basis = random_basis()

        # Если Eve угадала базис Alice, она точно узнаёт бит
        if eve_basis == photon.basis:
            self.basis_matches_with_alice += 1
            self.learned_bits += 1

        # Eve измеряет фотон в своём базисе
        measured_bit = measure_photon(photon, eve_basis)

        # После измерения Eve создаёт новый фотон
        # уже со своим измеренным битом и своим базисом
        new_photon = Photon.create(measured_bit, eve_basis)

        return new_photon

    """
        Возвращает статистику Eve.
    """

    def get_statistics(self) -> dict:
        
        if self.intercepted_count == 0:
            basis_match_rate = 0
        else:
            basis_match_rate = self.basis_matches_with_alice / self.intercepted_count

        return {
            "eve_intercepted_count": self.intercepted_count,
            "eve_basis_matches_with_alice": self.basis_matches_with_alice,
            "eve_basis_match_rate": basis_match_rate,
            "eve_learned_bits": self.learned_bits
        }