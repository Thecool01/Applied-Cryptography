from privacy import privacy_amplification, key_to_string

key = [1, 0, 1, 1, 0, 0]

final_key = privacy_amplification(key)

print("Original key:", key)
print("Final key:", final_key)
print("Final key as string:", key_to_string(final_key))