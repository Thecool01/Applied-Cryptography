from bb84_core import run_bb84


print("=== BB84 WITHOUT EVE ===")
result = run_bb84(
    num_photons=1000,
    check_percentage=0.1,
    error_threshold=0.11,
    eve_enabled=False,
    noise_rate=0.01
)

for key, value in result.items():
    print(key, ":", value)


print("\n=== BB84 WITH EVE ===")
result = run_bb84(
    num_photons=1000,
    check_percentage=0.1,
    error_threshold=0.11,
    eve_enabled=True,
    noise_rate=0.01
)

for key, value in result.items():
    print(key, ":", value)