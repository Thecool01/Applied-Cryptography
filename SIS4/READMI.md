# SIS4 — BB84 Quantum Key Distribution Simulator

## Students:
> Ishutin Nikolay,
> Urazakov Omar

This project is a classical simulation of the BB84 Quantum Key Distribution protocol.  
It demonstrates how Alice and Bob can generate a shared secret key using photon polarization, random measurement bases, basis reconciliation, error checking, and privacy amplification.

The project also includes bonus modules for the E91 protocol and Cascade error correction.

## Features

- BB84 protocol simulation
- Photon polarization encoding
- Quantum measurement simulation
- Basis reconciliation
- Error checking with threshold-based abort
- Eve intercept-resend attack simulation
- Privacy amplification using XOR pairs
- E91 protocol bonus simulation
- Cascade error correction bonus module
- Tkinter GUI
- Step-by-step BB84 visualization
- Auto Play / Pause mode
- Real-time statistics display
- Key evolution visualization
- Validation tests

## Project Structure

```text
SIS4/
├── photon.py        # Photon state, basis, polarization, measurement
├── eve.py           # Eve intercept-resend attack
├── privacy.py       # Privacy amplification
├── bb84_core.py     # Main BB84 protocol implementation
├── e91_core.py      # Bonus E91 protocol simulation
├── cascade.py       # Bonus Cascade error correction
├── gui.py           # Tkinter graphical interface
├── main.py          # Console demo runner
├── tests.py         # Validation tests
└── README.md        # Project documentation
```
## How to Run the GUI

Open the `SIS4` folder in terminal and run:

```
python gui.py
```

In the GUI, you can configure:

- Protocol: BB84 or E91
- Number of photons / pairs
- Error checking percentage
- Error threshold
- Channel noise rate
- Eve enabled / disabled

For BB84, you can use:

- `Run Simulation`
- `Next Step`
- `Auto Play`
- `Pause`
- `Reset Step`

## How to Run Console Demo

```
python main.py
```

This runs the protocol in console mode and prints the main statistics.

## How to Run Tests

```
python tests.py
```

The test suite checks:

- Same basis measurement
- Different basis measurement
- Basis match rate
- BB84 without Eve
- BB84 with Eve
- Key agreement
- Scalability with different photon counts
- Edge case with small photon count
- Multiple-run statistical behavior
- Information leakage analysis
- E91 bonus module
- Cascade bonus module