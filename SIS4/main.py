from bb84_core import run_bb84

def print_result(result: dict):


    print("\n==============================")
    print(f"Protocol: {result['protocol']}")
    print(f"Status: {result['status']}")
    print("==============================")

    print(f"Initial photons: {result['initial_count']}")
    print(f"Sifted key length: {result['sifted_key_length']}")
    print(f"Checked bits: {result['checked_bits']}")
    print(f"Errors found: {result.get('errors_found', 0)}")
    print(f"Error rate: {result['error_rate'] * 100:.2f}%")
    print(f"Basis match rate: {result.get('basis_match_rate', 0) * 100:.2f}%")

    print(f"Final key length: {result['final_key_length']}")
    print(f"Efficiency: {result['efficiency']:.2f}%")
    print(f"Eve enabled: {result['eve_enabled']}")
    print(f"Noise rate: {result['noise_rate'] * 100:.2f}%")

    if result["success"]:
        print(f"Final key preview: {result['final_key_preview']}")
        print(f"Keys match before amplification: {result['keys_match_before_amplification']}")

    if result["eve_enabled"]:
        print("\n--- Eve Statistics ---")
        print(f"Eve intercepted photons: {result.get('eve_intercepted_count', 0)}")
        print(f"Eve basis matches with Alice: {result.get('eve_basis_matches_with_alice', 0)}")
        print(f"Eve basis match rate: {result.get('eve_basis_match_rate', 0) * 100:.2f}%")
        print(f"Eve learned bits: {result.get('eve_learned_bits', 0)}")

"""
    Запускает две демонстрации:
    1. BB84 без Eve
    2. BB84 с Eve
"""

def run_demo():
    print("\nBB84 DEMO WITHOUT EVE")
    result_without_eve = run_bb84(
        num_photons=1000,
        check_percentage=0.1,
        error_threshold=0.11,
        eve_enabled=False,
        noise_rate=0.01
    )
    print_result(result_without_eve)

    print("\n\nBB84 DEMO WITH EVE")
    result_with_eve = run_bb84(
        num_photons=1000,
        check_percentage=0.1,
        error_threshold=0.11,
        eve_enabled=True,
        noise_rate=0.01
    )
    print_result(result_with_eve)


if __name__ == "__main__":
    run_demo()