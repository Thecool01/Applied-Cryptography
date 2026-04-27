"""
 Выполняет XOR двух битов.

 Правила XOR:
 0 XOR 0 = 0
 0 XOR 1 = 1
 1 XOR 0 = 1
 1 XOR 1 = 0
"""

def xor_bits(bit1: int, bit2: int) -> int:

    return bit1 ^ bit2


"""
    Упрощённая privacy amplification.

    Идея:
    - берём ключ после error checking;
    - разбиваем биты на пары;
    - каждую пару сжимаем в один бит через XOR.

    Пример:
    [1, 0, 1, 1] -> [1 XOR 0, 1 XOR 1] -> [1, 0]

    Так финальный ключ становится короче,
    но потенциальная информация Eve уменьшается.
"""

def privacy_amplification(key_bits: list[int]) -> list[int]:

    final_key = []

    # Идём по ключу с шагом 2
    for i in range(0, len(key_bits) - 1, 2):
        new_bit = xor_bits(key_bits[i], key_bits[i + 1])
        final_key.append(new_bit)

    return final_key

"""
  Преобразует список битов в строку.

  Пример:
  [1, 0, 1, 1] -> "1011"
"""
def key_to_string(key_bits: list[int]) -> str:

    return "".join(str(bit) for bit in key_bits)