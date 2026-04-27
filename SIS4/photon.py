import random
from dataclasses import dataclass


# Возможные базисы
# "+" — прямолинейный (rectilinear)
# "x" — диагональный (diagonal)
BASES = ["+", "x"]


def random_bit() -> int:
    """Генерирует случайный бит (0 или 1)"""
    return random.randint(0, 1)


def random_basis() -> str:
    """Случайно выбирает базис (+ или x)"""
    return random.choice(BASES)

"""
   Преобразует бит и базис в угол поляризации фотона.

   Прямолинейный базис (+):
       0 -> 0°
       1 -> 90°

   Диагональный базис (x):
       0 -> 45°
       1 -> 135°
"""

def get_polarization(bit_value: int, basis: str) -> int:


    if basis == "+":
        return 0 if bit_value == 0 else 90

    if basis == "x":
        return 45 if bit_value == 0 else 135

    raise ValueError("Базис должен быть '+' или 'x'")


"""
   Класс фотона (квантового состояния)

   bit_value — классический бит (0 или 1)
   basis — базис кодирования ('+' или 'x')
   polarization — угол поляризации (0, 45, 90, 135)
"""

@dataclass
class Photon:

    bit_value: int
    basis: str
    polarization: int

    """
        Создаёт фотон на основе бита и базиса
        автоматически вычисляя поляризацию
    """

    @classmethod
    def create(cls, bit_value: int, basis: str):

        polarization = get_polarization(bit_value, basis)
        return cls(bit_value, basis, polarization)

"""
    Симуляция измерения фотона (как делает Bob)

    Если базисы совпадают:
        результат всегда правильный

    Если базисы разные:
        результат случайный (0 или 1)
"""

def measure_photon(photon: Photon, measurement_basis: str) -> int:
    
    # Если базис совпадает → получаем правильный бит
    if photon.basis == measurement_basis:
        return photon.bit_value

    # Если базис не совпадает → случайный результат
    return random_bit()